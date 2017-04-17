#include "ae.h"

internal
auto AppUpdateAndRender(map<int, ControllerState> gamepadStates) -> void{
	float x = 0.0f;
	for(auto entry:gamepadStates){
		auto gamepadState = entry.second;
		x = gamepadState.axes[0];
		break;
	}
	//aeBusySleep(seconds(0.001f));
	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 0.0f);
	//glVertex3f(-0.6f, -0.4f, 0.0f);
	glVertex3f(x, -0.4f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, -0.4f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.6f, 0.0f);


	//glColor3f(1.0f, 0.0f, 0.0f);
	//glVertex3f(0.0f, 0.0f, 0.0f);

	//glColor3f(0.0f, 1.0f, 0.0f);
	//glVertex3f(500.0f, 200.0f, 0.0f);

	//glColor3f(0.0f, 0.0f, 1.0f);
	//glVertex3f(200.0f, 500.0f, 0.0f);

	glEnd();

}

