#include "ae.h"

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

	auto init_end_time = now();
	auto init_duration = init_end_time - start_time;

	Log::info("app",VARSTR(init_duration));

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	auto end_time = now();
	auto run_duration = end_time - start_time;
	Log::info("app",VARSTR(run_duration));

	return 0;
}

