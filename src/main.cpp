#include "ae.h"

using namespace std;

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
};

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

auto main(int argc, char** argv) -> int{
	auto start_time = now();

	auto console_logger_backend = new ConsoleLoggerBackend(start_time);
	Log::instance().add_backend(console_logger_backend);

	Log::info("app",VARSTR(start_time));

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

	bool keep_running = true;
	while (keep_running)
	{

		GameInput game_input;
		{
			KeyInput key_input;
			while(g_key_input_queue.pop(key_input)){
				Log::verbose( "input", String("popped ") + VARSTR(key_input));
			}
			for(auto joystick : joysticks) {
				auto joystick_state = get_joystick_state(joystick);
				//Log::verbose("input",VARSTR(joystick_state));
				//GameInput game_input {joystick_state};
				//return game_input;
				game_input = create_game_input(joystick_state);
				if(game_input.jump > 0.3){
					int i = 2;
				}

			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		if(glfwWindowShouldClose(window)){
			keep_running = false;
		}
		if(keep_running){
			busy_sleep(seconds(0.2));
		}
	}

	glfwTerminate();

	auto end_time = now();
	auto run_duration = end_time - start_time;
	Log::info("app",VARSTR(run_duration));

	return 0;
}

