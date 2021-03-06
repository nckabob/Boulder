#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "InputHandler.h"
#include "Singleton.h"

InputHandler::InputHandler(ControllableCharacter* pc) {
	player_character = pc;
	number_of_frames_to_eat_inputs = 0;
	UsingArrowsForMovement = true;
}

void InputHandler::EatInputsForNumberOfFrames(int number_of_frames) {
	number_of_frames_to_eat_inputs = number_of_frames;
}

void InputHandler::Update() {
	UsingArrowsForMovement = Singleton<Settings>::Get()->using_arrows_for_movement;

	bool a_button_current = false;
	bool b_button_current = false;
	bool x_button_current = false;
	bool y_button_current = false;

	bool right_bumper_button_current = false;
	bool left_bumper_button_current = false;
	bool select_button_current = false;
	bool start_button_current = false;

	float left_stick_horizontal = 0.0f;
	float left_stick_vertical = 0.0f;

	float right_stick_horizontal = 0.0f;
	float right_stick_vertical = 0.0f;

	//right trigger is the negative one
	float right_trigger = 0.0f;
	float left_trigger = 0.0f;

	float dpad_horizontal = 0.0f;
	float dpad_vertical = 0.0f;

	float mystery = 0.0f;

	if (sf::Joystick::isConnected(player_character->GetPlayerIndex())) {
		a_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), a_button);
		b_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), b_button);
		x_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), x_button);
		y_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), y_button);

		right_bumper_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), right_bumper_button);
		left_bumper_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), left_bumper_button);
		select_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), select_button);
		start_button_current = sf::Joystick::isButtonPressed(player_character->GetPlayerIndex(), start_button);

		left_stick_horizontal = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::X);
		left_stick_vertical = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::Y);

		right_stick_horizontal = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::U);
		right_stick_vertical = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::R);

		right_trigger = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::Z);
		left_trigger = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::Z);

		dpad_horizontal = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::PovX);
		dpad_vertical = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::PovY);

		mystery = sf::Joystick::getAxisPosition(player_character->GetPlayerIndex(), sf::Joystick::V);
	} else {
		a_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		b_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
		x_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
		y_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);

		right_bumper_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
		left_bumper_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
		select_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
		start_button_current = sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

		if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::Right) : sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			left_stick_horizontal = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? 50.0f : 100.0f;
		} else if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::Left) : sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			left_stick_horizontal = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? -50.0f : -100.0f;
		}

		if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::Up) : sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			left_stick_vertical = -100.0f;
		} else if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::Down) : sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			left_stick_vertical = 100.0f;
		}


		if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::D) : sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			right_stick_horizontal = 100.0f;
		} else if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::A) : sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			right_stick_horizontal = -100.0f;
		}

		if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::W) : sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			right_stick_vertical = -100.0f;
		} else if (UsingArrowsForMovement ? sf::Keyboard::isKeyPressed(sf::Keyboard::S) : sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			right_stick_vertical = 100.0f;
		}


		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3)) {
			dpad_horizontal = 100.0f;
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) {
			dpad_horizontal = -100.0f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad5)) {
			dpad_vertical = -100.0f;
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) {
			dpad_vertical = 100.0f;
		}
	}

	if (number_of_frames_to_eat_inputs > 0) {
		number_of_frames_to_eat_inputs--;
	} else if (player_character->GetHitPoints() > 0 && player_character->CanTakeInput()) {
		if (a_button_current && !a_button_previous) {
			player_character->HandleButtonAPress();
		}
		if (!a_button_current && a_button_previous) {
			player_character->HandleButtonARelease();
		}
		
		if (b_button_current && !b_button_previous) {
			player_character->HandleButtonBPress();
		}
		if (!b_button_current && b_button_previous) {
			player_character->HandleButtonBRelease();
		}
		
		if (x_button_current && !x_button_previous) {
			player_character->HandleButtonXPress();
		}
		if (!x_button_current && x_button_previous) {
			player_character->HandleButtonXRelease();
		}
		
		if (y_button_current && !y_button_previous) {
			player_character->HandleButtonYPress();
		}
		if (!y_button_current && y_button_previous) {
			player_character->HandleButtonYRelease();
		}
		
		
		if (right_bumper_button_current && !right_bumper_button_previous) {
			player_character->HandleButtonRightBumperPress();
		}
		if (!right_bumper_button_current && right_bumper_button_previous) {
			player_character->HandleButtonRightBumperRelease();
		}
		
		if (left_bumper_button_current && !left_bumper_button_previous) {
			player_character->HandleButtonLeftBumperPress();
		}
		if (!left_bumper_button_current && left_bumper_button_previous) {
			player_character->HandleButtonLeftBumperRelease();
		}
		
		if (select_button_current && !select_button_previous) {
			player_character->HandleButtonSelectPress();
		}
		if (!select_button_current && select_button_previous) {
			player_character->HandleButtonSelectRelease();
		}
		
		if (start_button_current && !start_button_previous) {
			player_character->HandleButtonStartPress();
		}
		if (!start_button_current && start_button_previous) {
			player_character->HandleButtonStartRelease();
		}
		
		if ((left_stick_horizontal >= deadzone || left_stick_horizontal <= -deadzone) ||
			(left_stick_vertical >= deadzone || left_stick_vertical <= -deadzone)) {
			player_character->HandleLeftStickInput(left_stick_horizontal, left_stick_vertical);
		} else {
			player_character->HandleLeftStickInput(0.0f, 0.0f);
		}

		if ((right_stick_horizontal >= deadzone || right_stick_horizontal <= -deadzone) ||
			(right_stick_vertical >= deadzone || right_stick_vertical <= -deadzone)) {
			player_character->HandleRightStickInput(right_stick_horizontal, right_stick_vertical);
		} else {
			player_character->HandleRightStickInput(0.0f, 0.0f);
		}

		if (dpad_horizontal == 100 && dpad_horizontal_previous != 100) {
			player_character->HandleDpadRightPress();
		} else if (dpad_horizontal == -100 && dpad_horizontal_previous != -100) {
			player_character->HandleDpadLeftPress();
		}

		if (dpad_vertical == 100 && dpad_vertical_previous != 100) {
			player_character->HandleDpadUpPress();
		} else if (dpad_vertical == -100 && dpad_vertical_previous != -100) {
			player_character->HandleDpadDownPress();
		}

		if (dpad_horizontal_previous == 100 && dpad_horizontal != 100) {
			player_character->HandleDpadRightRelease();
		} else if (dpad_horizontal_previous == -100 && dpad_horizontal != -100) {
			player_character->HandleDpadLeftRelease();
		}

		if (dpad_vertical_previous == 100 && dpad_vertical != 100) {
			player_character->HandleDpadUpRelease();
		} else if (dpad_vertical_previous == -100 && dpad_vertical != -100) {
			player_character->HandleDpadDownRelease();
		}
	}

	a_button_previous = a_button_current;
	b_button_previous = b_button_current;
	x_button_previous = x_button_current;
	y_button_previous = y_button_current;

	dpad_horizontal_previous = dpad_horizontal;
	dpad_vertical_previous = dpad_vertical;

	right_bumper_button_previous = right_bumper_button_current;
	left_bumper_button_previous = left_bumper_button_current;
	select_button_previous = select_button_current;
	start_button_previous = start_button_current;
}

//int InputHandler::GetAvailablePlayerIndex() {
//	int return_value = next_player_index;
//
//	next_player_index++;
//
//	return return_value;
//}