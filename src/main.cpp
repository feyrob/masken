#include "ae.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <dlfcn.h>
#include <cmath>

using namespace std;

typedef ALCdevice* F_alcOpenDevice(const ALCchar* devicename);
typedef ALCcontext* F_alcCreateContext(ALCdevice *device, const ALCint* attrlist);
typedef ALCboolean F_alcMakeContextCurrent(ALCcontext *context);
typedef void F_alListenerfv(ALenum param, const ALfloat *values);
typedef void F_alGenBuffers(ALsizei n, ALuint *buffers);
typedef void F_alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
typedef void F_alGenSources(ALsizei n, ALuint *sources);
typedef void F_alSourcei(ALuint source, ALenum param, ALint value);
typedef void F_alSourcePlay(ALuint source);

CircularFifo<KeyInput, 16> g_key_input_queue;

const double c_pi = 3.14159265358979323846;


U64 cyc(){
	U32 lo;
	U32 hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((U64)hi << 32) | lo;
}

static void key_callback(
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods
) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	auto press_time = now();
	KeyInput key_input{press_time,key,scancode,action,mods};
	bool queue_success = g_key_input_queue.push(key_input);
	if(!queue_success){
		Log::warning("input","g_key_input_queue full");
	}
}

struct GameInput{
	float x;
	float y;
	float jump;
	String to_string(){
		String m("{");
		m+= VARSTR3(x,1,2) + " " + VARSTR3(y,1,2) + " " + VARSTR3(jump,1,1);
		m+= "}";
		return m;
	}
	//bool is_equal(GameInput rhs){
    //return ( 0 == std::memcmp( this, &rhs, sizeof( GameInput ) ) );
	//}
	bool is_equal(GameInput rhs){
		bool b_equal = (x == rhs.x) && (y == rhs.y) && (jump == rhs.jump);
		return b_equal;
	}
};

static String str(GameInput gi){
	return gi.to_string();
}

GameInput createGameInput(GamepadState state){
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

void gamepadCallback(int gamepadId, int event) {
	// NOTE (feyrob) this seems to be called very unreliably
	Log::info("input","gamepad event");
	GamepadConnectionEvent connectionEvent{gamepadId,event};
	bool isSuccess = g_gamepadConnectionEventQueue.push(connectionEvent);
	if(!isSuccess){
		Log::warning("input","g_gamepadConnectionEventQueue full");
	}
}

vector<int> getGamepadIdList(){
	vector<int> connectedGamepadIdList;
	for(int id = 0; id <= GLFW_JOYSTICK_LAST ; ++id){
		int present = glfwJoystickPresent(id);
		if(present){
			connectedGamepadIdList.push_back(id);
		}
	}
	return connectedGamepadIdList;
}

void printGamepadList(vector<int> gamepadIdList){
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

int
main(int argc, char** argv) {

	auto start_time = now();


	auto console_logger_backend = new ConsoleLoggerBackend(start_time);
	Log::instance().add_backend(console_logger_backend);

	Log::info("app",VARSTR(start_time));

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


	ALCdevice* audio_device = f_alcOpenDevice(nullptr);
	if(audio_device){
		Log::verbose("audio","device opening success");
	}else{
		Log::warning("audio","device opening failure");
	}

	F_alcCreateContext* f_alcCreateContext = nullptr;
	f_alcCreateContext = (F_alcCreateContext*)dlsym(handle, "alcCreateContext");
	ALCcontext* audio_context = f_alcCreateContext(audio_device, NULL);
	if(audio_context){
		Log::verbose("audio","context creation success");
	}else{
		Log::warning("audio","context creation failure");
	}

	F_alcMakeContextCurrent* f_alcMakeContextCurrent = nullptr;
	f_alcMakeContextCurrent = (F_alcMakeContextCurrent*)dlsym(handle, "alcMakeContextCurrent");

	auto success = f_alcMakeContextCurrent(audio_context);
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
			samples[i] = 32760 * sin( (2.f*float(c_pi)*freq)/sample_rate * i );
	}
	f_alBufferData(buf1, AL_FORMAT_MONO16, samples, buf_size, sample_rate);

	ALuint src = 0;
	f_alGenSources(1, &src);
	f_alSourcei(src, AL_BUFFER, buf1);
	f_alSourcePlay(src);

	GLFWwindow* window;
	if (!glfwInit()){
			return -1;
	}

	window = glfwCreateWindow(1024,768, "masken", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	auto gamepadIdList = getGamepadIdList();
	printGamepadList(gamepadIdList);

	glfwSetJoystickCallback(gamepadCallback);

	auto init_end_time = now();
	auto init_duration = init_end_time - start_time;

	Log::info("app",VARSTR(init_duration));

	GameInput gameInput;
	U64 frame_idx(0);
	bool keep_running = true;

	U64 previousCyc = cyc();

	while (keep_running)
	{

		{
			GameInput newGameInput;
			KeyInput key_input;
			while(g_key_input_queue.pop(key_input)){
				Log::verbose( "input", String("popped ") + VARSTR(key_input));
			}
			for(auto gamepadId: gamepadIdList) {
				auto gamepadState = getGamepadState(gamepadId);
				newGameInput = createGameInput(gamepadState);
				if(gameInput.is_equal(newGameInput)){
					// nothing
				} else{
					Log::verbose("input/game", VARSTR(gameInput) + String(" -> ") + VARSTR(newGameInput));
					gameInput = newGameInput;

					f_alSourcePlay(src);
				}

			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		if(glfwWindowShouldClose(window)){
			keep_running = false;
		}
		if(keep_running){
			busy_sleep(seconds(0.005));
		}
		++frame_idx;
		U64 currentCyc = cyc();

		U64 deltaCyc = currentCyc - previousCyc;
		U64 deltaCyc3 = deltaCyc / 1000;

		//Log::debug("cyc",VARSTR(deltaCyc3));
		previousCyc = currentCyc;
	}

	glfwTerminate();

	auto end_time = now();
	auto run_duration = end_time - start_time;
	Log::info("app",VARSTR(run_duration));


	return 0;
}

