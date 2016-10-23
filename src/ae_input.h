#pragma once
#include "platform.h"
#include "ae_string.h"

char const* key_action_cstr[] = {
	"release",
	"press",
	"repeat"
};

struct KeyInput{
	Time time;
	int key;
	int scancode;
	int action;
	int mods;

	String to_string(){
		String m("{ ");
		m+= VARSTR(time) + " ";
		m+= VARSTR2(key,4) + " ";
		const char* key_name = glfwGetKeyName(key, scancode);
		m+= "key_name:";
		if(key_name){
			m+= "'";
			m+= key_name;
			m+= "'";
		}else{
			m+= "nil";
		}
		m+= " ";
		m+= VARSTR2(scancode,4) + " ";
		m+= "action:";
		String a(key_action_cstr[action]);
		while(a.size() < 7){
			a += " ";
		}
		m+= a;
		m+= " ";
		m+= VARSTR2(mods,2) + " }";
		return m;
	}
};

static String str(KeyInput input){
	String m = input.to_string();
	return m;
}

struct JoystickState{
	std::vector<float> axes;
	std::vector<U8> buttons;
	String to_string(){
		String m("axes:{");
		for(int i = 0; i<axes.size();++i){
			m+= str(i);
			m+= ":";
			m+= str(axes[i]);
			m+= " ";
		}
		m+="} ";
		m+= "buttons:{";
		for(int i = 0; i<buttons.size();++i){
			m+= str(i);
			m+= ":";
			m+= str(buttons[i]);
			m+= " ";
		}
		m+= "}";
		return m;
	}
};

static String str(JoystickState state){
	String m = state.to_string();
	return m;
}

JoystickState get_joystick_state(int joystick){
	int axes_count;
	const float* axes = glfwGetJoystickAxes(joystick, &axes_count);
	std::vector<float> axes_vec;
	axes_vec.reserve(axes_count);
	axes_vec.assign(axes, axes + axes_count);

	int button_count;
	const unsigned char* buttons = glfwGetJoystickButtons(joystick, &button_count);
	std::vector<U8> button_vec;
	button_vec.reserve(button_count);
	button_vec.assign(buttons, buttons + button_count);

	JoystickState state{axes_vec, button_vec};
	return state;
}

