#pragma once
#include "platform.h"
#include "ae_string.h"

// TODO(feyrob) move into KeyInput
char const* c_action_to_name[] = {
	"release",
	"press",
	"repeat"
};

struct KeyInput{
	TTime time;
	int key;
	int scancode;
	int action;
	int mods;


	String ToString(){
		String m("{ ");
		m+= VARSTR(time) + " ";
		m+= VARSTR2(key,4) + " ";
		const char* keyName = glfwGetKeyName(key, scancode);
		m+= "keyName:";
		if(keyName){
			m+= "'";
			m+= keyName;
			m+= "'";
		}else{
			m+= "nil";
		}
		m+= " ";
		m+= VARSTR2(scancode,4) + " ";
		m+= "action:";
		String a(c_action_to_name[action]);
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
	String m = input.ToString();
	return m;
}

struct ControllerState{
	std::vector<float> axes;
	std::vector<U8> buttons;

	bool isEqual(ControllerState rhs){
		if(axes.size() != rhs.axes.size()){
			return false;
		}
		if(buttons.size() != rhs.buttons.size()){
			return false;
		}
		for(int i=0;i<axes.size();++i){
			if(axes[i] != rhs.axes[i]){
				return false;
			}
		}
		for(int i=0;i<buttons.size();++i){
			if(buttons[i] != rhs.buttons[i]){
				return false;
			}
		}
		return true;
	}

	String ToString(){
		String m("{");
		//m += (VARSTR(gamepadId));
		//m += " ";
		m += "axes:{";
		for(int i = 0; i<axes.size();++i){
			if(i>0){
				m+= " ";
			}
			m+= str(i);
			m+= ":";
			m+= str3(axes[i],1,2);
		}
		m+="} ";
		m+= "btns:{";
		for(int i = 0; i<buttons.size();++i){
			if(i>0){
				m+= " ";
			}
			m+= str(i);
			m+= ":";
			m+= str(buttons[i]);
		}
		m+= "}}";
		return m;
	}
};

struct ControllerInputEvent{
	TTime time;
	ControllerState state;
};

static String str(ControllerState state){
	String m = state.ToString();
	return m;
}

ControllerState GetControllerState(int gamepadId){
	int axesCount;
	const float* axes = glfwGetJoystickAxes(gamepadId, &axesCount);
	std::vector<float> axesVec;
	axesVec.reserve(axesCount);
	axesVec.assign(axes, axes + axesCount);

	int buttonCount;
	const unsigned char* buttons = glfwGetJoystickButtons(gamepadId, &buttonCount);
	std::vector<U8> buttonList;
	buttonList.reserve(buttonCount);
	buttonList.assign(buttons, buttons + buttonCount);

	ControllerState state{axesVec, buttonList};
	return state;
}

