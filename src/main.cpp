#include "ae.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <dlfcn.h>

using namespace std;

typedef ALCdevice* F_alcOpenDevice(const ALCchar* devicename);

CircularFifo<KeyInput, 16> g_key_input_queue;

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

GameInput create_game_input(JoystickState joystick_state){
	float x = joystick_state.axes[0];
	float y = joystick_state.axes[1];
	float jump = static_cast<float>(joystick_state.buttons[1]);
	GameInput game_input{x,y,jump};
	return game_input;
}

struct JoystickConnectionEvent{
	int joy;
	int event;
};

CircularFifo<JoystickConnectionEvent, 4> g_joystick_queue;

void joystick_callback(int joy, int event) {
	// NOTE (feyrob) this seems to be called very unreliably
	Log::info("input","joystick event");
	JoystickConnectionEvent joystick_event{joy,event};
	bool is_success = g_joystick_queue.push(joystick_event);
	if(!is_success){
		Log::warning("input","g_joystick_queue full");
	}
}

vector<int> get_joysticks(){
	vector<int> connected_joysticks;
	for(int id = 0; id <= GLFW_JOYSTICK_LAST ; ++id){
		int present = glfwJoystickPresent(id);
		if(present){
			connected_joysticks.push_back(id);
		}
	}
	return connected_joysticks;
}

void print_joysticks(vector<int> joysticks){
	if(joysticks.size() == 0){
		Log::warning("input","no joysticks found");
	}
	for(auto joystick : joysticks) {
		int axes_count;
		const float* axes = glfwGetJoystickAxes(joystick, &axes_count);

		int button_count;
		const unsigned char* buttons = glfwGetJoystickButtons(joystick, &button_count);

		const char* name = glfwGetJoystickName(joystick);
		Log::verbose("input", VARSTR2(joystick,2) + " " + VARSTR2(axes_count,2) + " " + VARSTR2(button_count,2) + " " + VARSTR(name));
	}
}

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

	//ALCdevice* device = alcOpenDevice(nullptr);
	ALCdevice* audio_device = f_alcOpenDevice(nullptr);
	if(audio_device){
		Log::verbose("audio","device open success");
	}else{
		Log::warning("audio","device open failure");
	}



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

	auto joysticks = get_joysticks();
	print_joysticks(joysticks);

	glfwSetJoystickCallback(joystick_callback);

	auto init_end_time = now();
	auto init_duration = init_end_time - start_time;

	Log::info("app",VARSTR(init_duration));

	GameInput game_input;
	U64 frame_idx(0);
	bool keep_running = true;
	while (keep_running)
	{

		{
			GameInput new_game_input;
			KeyInput key_input;
			while(g_key_input_queue.pop(key_input)){
				Log::verbose( "input", String("popped ") + VARSTR(key_input));
			}
			for(auto joystick : joysticks) {
				auto joystick_state = get_joystick_state(joystick);
				new_game_input = create_game_input(joystick_state);
				if(game_input.is_equal(new_game_input)){
					// nothing
				} else{
					Log::verbose("input/game", VARSTR(game_input) + String(" -> ") + VARSTR(new_game_input));
					game_input = new_game_input;
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
	}

	glfwTerminate();

	auto end_time = now();
	auto run_duration = end_time - start_time;
	Log::info("app",VARSTR(run_duration));

	return 0;
}

