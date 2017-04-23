#include "ae.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <dlfcn.h>
#include <cmath>
#include <map>

using namespace std;

typedef ALCdevice* F_alcOpenDevice(const ALCchar* devicename);
typedef void F_alcCloseDevice(ALCdevice* device);
typedef ALCcontext* F_alcCreateContext(ALCdevice *device, const ALCint* attrlist);
typedef ALCboolean F_alcMakeContextCurrent(ALCcontext *context);
typedef void F_alListenerfv(ALenum param, const ALfloat *values);
typedef void F_alGenBuffers(ALsizei n, ALuint *buffers);
typedef void F_alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
typedef void F_alGenSources(ALsizei n, ALuint *sources);
typedef void F_alSourcei(ALuint source, ALenum param, ALint value);
typedef void F_alSourcePlay(ALuint source);

#include "app.cpp"

CircularFifo<KeyInput, 16> g_KeyInputQueue;

// TODO(feyrob) get from some math header?
global const double c_pi = 3.14159265358979323846;


internal void 
AeBusySleep(TDuration duration){
	TTime startTime = Now();
	TTime targetTime = startTime + duration;
	while(true){
		TTime currentTime = Now(); 
		if(currentTime < targetTime){
			// keep sleeping
		}else{
			return;
		}
	}
}

U64 
cyc(){
	U32 lo;
	U32 hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((U64)hi << 32) | lo;
}

internal void 
KeyEventCallback(
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods
) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	auto pressTime = Now();
	KeyInput keyInput{pressTime, key, scancode, action,mods};
	bool queueSuccess = g_KeyInputQueue.push(keyInput);
	if(!queueSuccess){
		Log::warning("input","g_KeyInputQueue full");
	}
}

struct GameInput{
	float x;
	float y;
	float jump;
	String toString(){
		String m("{");
		m+= VARSTR3(x,1,2) + " " + VARSTR3(y,1,2) + " " + VARSTR3(jump,1,1);
		m+= "}";
		return m;
	}
	bool isEqual(GameInput rhs){
		bool b_equal = (x == rhs.x) && (y == rhs.y) && (jump == rhs.jump);
		return b_equal;
	}
};

internal String 
str(GameInput gi){
	return gi.toString();
}

GameInput 
createGameInput(ControllerState state){
	float x = state.axes[0];
	float y = state.axes[1];
	float jump = static_cast<float>(state.buttons[1]);
	GameInput gameInput{x,y,jump};
	return gameInput;
}

struct GamepadConnectionEvent{
	int gamepadId;
	int event;
};

CircularFifo<GamepadConnectionEvent, 4> g_gamepadConnectionEventQueue;

void
gamepadConnectionEventCallback(int gamepadId, int event) {
	// NOTE (feyrob) this seems to be called very unreliably
	Log::info("input","gamepad event");
	GamepadConnectionEvent connectionEvent{gamepadId,event};
	bool isSuccess = g_gamepadConnectionEventQueue.push(connectionEvent);
	if(!isSuccess){
		Log::warning("input","g_gamepadConnectionEventQueue full");
	}
}

vector<int> 
getGamepadIdList(){
	vector<int> connectedGamepadIdList;
	for(int id = 0; id <= GLFW_JOYSTICK_LAST ; ++id){
		int present = glfwJoystickPresent(id);
		if(present){
			connectedGamepadIdList.push_back(id);
		}
	}
	return connectedGamepadIdList;
}

void 
printGamepadList(vector<int> gamepadIdList){
	if(gamepadIdList.size() == 0){
		Log::warning("input","no gamepads found");
	}
	for(auto gamepadId: gamepadIdList) {
		int axesCount;
		const float* axes = glfwGetJoystickAxes(gamepadId, &axesCount);

		int buttonCount;
		const unsigned char* buttons = glfwGetJoystickButtons(gamepadId, &buttonCount);

		const char* name = glfwGetJoystickName(gamepadId);
		Log::verbose("input", VARSTR2(gamepadId,2) + " " + VARSTR2(axesCount,2) + " " + VARSTR2(buttonCount,2) + " " + VARSTR(name));
	}
}


const int c_audioBufferCount= 7;

ALuint	g_audioBuffers[c_audioBufferCount];

map<int,ControllerState> 
GetControllerStates(){
	auto gamepadIdList = getGamepadIdList();

	map<int,ControllerState> states;
	for(auto gamepadId: gamepadIdList) {
		auto state = GetControllerState(gamepadId);
		states[gamepadId] = state;
	}
	return states;
}


int
main(int argc, char** argv) {

	// count initialization time as time of first frame
	U64 frameStartCyc = cyc();
	auto startTime = Now();
	TTime frameStartTime = startTime;
	
	TTime nextPerfPrintTime = startTime + Seconds(1);

	auto consoleLoggerBackend = new ConsoleLoggerBackend(startTime);
	Log::instance().addBackend(consoleLoggerBackend);

	Log::info("app",VARSTR(startTime));

	void* handle = nullptr;
	handle = dlopen("/usr/lib/libopenal.so", RTLD_LAZY);
	if(handle){
		Log::verbose("audio","success loading libopenal.so");
	}else{
		Log::warning("audio","failure loading libopenal.so");
	}
	// clean existing errors if any
	dlerror();

	F_alcOpenDevice* f_alcOpenDevice = nullptr;
	f_alcOpenDevice = (F_alcOpenDevice*)dlsym(handle, "alcOpenDevice");

	F_alcCloseDevice* f_alcCloseDevice = nullptr;
	f_alcCloseDevice = (F_alcCloseDevice*)dlsym(handle, "alcCloseDevice");

	ALCdevice* audioDevice = f_alcOpenDevice(nullptr);
	if(audioDevice){
		Log::verbose("audio","device opening success");
	}else{
		Log::warning("audio","device opening failure");
	}

	F_alcCreateContext* f_alcCreateContext = nullptr;
	f_alcCreateContext = (F_alcCreateContext*)dlsym(handle, "alcCreateContext");
	ALCcontext* audioContext = f_alcCreateContext(audioDevice, NULL);
	if(audioContext){
		Log::verbose("audio","context creation success");
	}else{
		Log::warning("audio","context creation failure");
	}

	F_alcMakeContextCurrent* f_alcMakeContextCurrent = nullptr;
	f_alcMakeContextCurrent = (F_alcMakeContextCurrent*)dlsym(handle, "alcMakeContextCurrent");

	auto success = f_alcMakeContextCurrent(audioContext);
	if(success){
		Log::verbose("audio","making context current success");
	}else{
		Log::warning("audio","making context current failure");
	}


	F_alListenerfv* f_alListenerfv = (F_alListenerfv*)dlsym(handle, "alListenerfv");
	F_alGenBuffers* f_alGenBuffers = (F_alGenBuffers*)dlsym(handle, "alGenBuffers");
	F_alBufferData* f_alBufferData = (F_alBufferData*)dlsym(handle, "alBufferData");

	F_alGenSources* f_alGenSources = (F_alGenSources*)dlsym(handle, "alGenSources");
	F_alSourcei* f_alSourcei = (F_alSourcei*)dlsym(handle, "alSourcei");
	F_alSourcePlay* f_alSourcePlay = (F_alSourcePlay*)dlsym(handle, "alSourcePlay");

	ALfloat listenerPos[]={0.0,0.0,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat	listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0};	// Listener facing into the screen

	f_alListenerfv(AL_POSITION,listenerPos); 	// Position ...
	f_alListenerfv(AL_VELOCITY,listenerVel); 	// Velocity ...
	f_alListenerfv(AL_ORIENTATION,listenerOri); 	// Orientation ...
	f_alGenBuffers(c_audioBufferCount, g_audioBuffers);  // Generate Buffers

	ALuint buf1 = g_audioBuffers[0];

	float freq = 440.0f;
	int secondCount = 4;
	unsigned sample_rate = 22050;
	size_t buf_size = secondCount * sample_rate;

	short *samples;
	samples = new short[buf_size];
	for(int i=0; i<buf_size; ++i) {
			samples[i] = 32760 * sin( (2.0f*float(c_pi)*freq)/sample_rate * i );
	}
	f_alBufferData(buf1, AL_FORMAT_MONO16, samples, buf_size, sample_rate);

	ALuint src = 0;
	f_alGenSources(1, &src);
	f_alSourcei(src, AL_BUFFER, buf1);

	GLFWwindow* window;
	if (!glfwInit()){
			return -1;
	}

	//GLuint vertex_buffer;
	//GLuint vertex_shader;
	//GLuint fragment_shader;
	//GLuint program;
	//GLint mvp_location; 
	//GLint vpos_location;
	//GLint vcol_location;

	window = glfwCreateWindow(1024,768, "masken", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// 0 vsync off
	// 1 vsync on
	glfwSwapInterval(0);

	glfwSetKeyCallback(window, KeyEventCallback);
	glfwSetJoystickCallback(gamepadConnectionEventCallback);

	auto initEndTime = Now();
	auto initDuration = initEndTime - startTime;

	Log::attention("app",VARSTR(initDuration));

	bool keep_running = true;


	//float pos = 0.0f;
	
	// can be removed once the game service handles inputs
	map<int,ControllerState> prevControllerStates;

	vector<U64> frameCycs;
	vector<TDuration> frameDurations;


	U64 frameIdx(0);

	U64 perfWindowStartIdx = frameIdx;
	//U64 worstFrameCyc = 0;
	//Duration worstFrameDuration = 0;

	while (keep_running)
	{
		auto currentControllerStates = GetControllerStates();
		for(auto entry : currentControllerStates){
			auto gamepadId = entry.first;
			auto gamepadState = entry.second;

			auto prevStateIter = prevControllerStates.find(gamepadId);
			bool same = true;
			if(prevStateIter != prevControllerStates.end()){
				same = gamepadState.isEqual(prevStateIter->second);
			}else{
				same = false;
			}

			if(!same){
				Log::verbose("input",VARSTR(gamepadState));
			}
		}

		//for(auto state: currentControllerStates){
			//pos += state.second.axes[0];
		//}



		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
		glViewport(0, 0, width, height);
		//glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//glOrtho(-ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
		//glOrtho(0, width, height, 0, 1.0f, -1.0f);
		//glOrtho(0, width, 0, height, 1.0f, -1.0f);
		glOrtho(-1.0, +1.0, -1.0, +1.0, 1.0f, -1.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glRotatef(pos * 50.0f, 0.0f, 0.0f, 1.0f);

		AppUpdateAndRender(currentControllerStates);

		//if(pos > 0.5f){
			//f_alSourcePlay(src);
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
		if(glfwWindowShouldClose(window)){
			keep_running = false;
		}
		if(keep_running){
			//AeBusySleep(Seconds(0.005));
		}

		U64 frameEndCyc = cyc();
		TTime frameEndTime = Now();

		auto frameCyc = frameEndCyc - frameStartCyc;
		auto frameDuration = frameEndTime - frameStartTime;

		if(nextPerfPrintTime < frameEndTime){
			U64 worstWindowFrameCyc(0);
			TDuration worstWindowFrameDuration = Seconds(0);
			for(int i = perfWindowStartIdx;i<frameIdx;++i){
				if(worstWindowFrameCyc < frameCycs[i]){
					worstWindowFrameCyc = frameCycs[i];
				}
				if(worstWindowFrameDuration < frameDurations[i]){
					worstWindowFrameDuration = frameDurations[i];
				}
			}

			// only look at the worst frame
			U64 fps = Seconds(1) / worstWindowFrameDuration;
			// 1.5 Ghz / 1 second [~1 000 000 000 nanos] = 1.5
			// 1.5 cyc ~= 1 nano ??

			//auto ratio = float(worstWindowFrameCyc) / float(worstWindowFrameDuration.count());
			//Log::debug("x",VARSTR(ratio));
			// prints ~1.5

			// 1hz                second 
			// 1khz 1 000         milli
			// 1mhz 1 000 000     micro
			// 1ghz 1 000 000 000 nano

			Log::verbose("perf",VARSTR(frameIdx) + " " + VARSTR(worstWindowFrameCyc) + " " + VARSTR(worstWindowFrameDuration) + " (" + VARSTR(fps)+  ")");

			nextPerfPrintTime = nextPerfPrintTime + Seconds(1);
			perfWindowStartIdx = frameIdx;
		}

		frameCycs.push_back(frameCyc);
		frameDurations.push_back(frameDuration);

		// use the end time of the current frame as start time of the next frame
		frameStartCyc = frameEndCyc;
		frameStartTime = frameEndTime;

		prevControllerStates = currentControllerStates;
		++frameIdx;
	}

	glfwTerminate();

	f_alcCloseDevice(audioDevice);

	auto end_time = Now();
	auto run_duration = end_time - startTime;
	Log::info("app",VARSTR(run_duration));


	return 0;
}

