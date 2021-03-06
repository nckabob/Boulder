#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "SmashWorld.h"
#include "Constants.h"

typedef void(*callback_function)(void); // type for conciseness

SmashWorld::SmashWorld() {
	timeSkateBoardWasPutAway = 0;
	timeTravelingMusicPlaysForAfterSkateboardIsPutAway = 300;
	timeAnEnemyWasLastNearby = 0;
}

void SmashWorld::Init(sf::RenderWindow* window, Camera* cam, float frames_per_second, int save_slot, bool load_game) {
	render_window = window;
	camera = cam;
	timeStep = 1.0f / frames_per_second;
	past_setup = false;
	saveSlot = save_slot;

	framesBetweenDialogueCharactersAppearing = 10;
	frameOfLastDialogueCharacterAppeared = 0;
	doesPlayerHaveHeadphonesOn = true;
	dialogueStringThatIsBeingSaid = "";
	dialogueStringThatIsAppearing = "";
	numberOfDialogueCharactersBeingDisplayed = 0;

	//if (!shader.loadFromFile("Shaders/colorShading.vert", "Shaders/colorShading.frag"))
	//{
	//	cout << "Unable to find colorShading.vert\n";
	//}

	//if (!shader.loadFromFile("Shaders/colorShading.frag", sf::Shader::Fragment))
	//{
	//	cout << "Unable to find colorShading.frag\n";
	//}
	if (!shader.loadFromFile("Shaders/BackgroundWarp.frag", sf::Shader::Fragment))
	{
		cout << "Unable to find BackgroundWarp.frag\n";
	}

	shader.setUniform("texture", sf::Shader::CurrentTexture);

	//if (!lighting_shader.loadFromFile("Shaders/LightingShader.frag", sf::Shader::Fragment))
	//{
	//	cout << "Unable to find LightingShader.frag\n";
	//}
	//lighting_shader.setUniform("lightColor", sf::Glsl::Vec4(0.0f, 1.0f, 1.0f, 0.5f));

	StageOfTheGame = STAGE_OF_GAME_INTRO;
	goToCredits = false;

	player_menu_input = new InputHandler(new MenuController(0));

#ifdef _DEBUG
	//string intro_cut_scene_file_relative_path = "Cutscenes/ShadowsDieTwice.avi";
	//if (!intro_cutscene.openFromFile(intro_cut_scene_file_relative_path))
	//{
	//	cout << "Unable to open file " << intro_cut_scene_file_relative_path << "\n";
	//}
	//
	//intro_cutscene.fit(0, 0, camera->viewport_dimensions.x, camera->viewport_dimensions.y);
	////intro_cutscene.play();
	//intro_cutscene.setVolume(0);
#else
	//intro_cutscene.play();
	//intro_cutscene.setVolume(100);
#endif

	player_menu_input->EatInputsForNumberOfFrames(1);

	ringbearer_font.loadFromFile("Images/RingbearerFont.ttf");

	render_window->setActive(false);

	setupThread = std::thread(&SmashWorld::UpdateLoadingScreen, this);

	Setup(); 
	
	if (load_game) {
		ImportSaveData(saveSlot);
	} else {
		ExportSaveData();
	}

	setupThread.join();

	render_window->setActive(true);
}

void SmashWorld::ExportSaveData() {
	//string save_data_file_name = "save_data.txt";
	//ifstream f(save_data_file_name.c_str());
	//if (!f.good()) {
	//
	//}

	ofstream ofs("save" + to_string(saveSlot) + ".sav", ios::binary | ios::out);

	Json::Value save_data;

	save_data["WorldData"]["StageOfTheGame"] = StageOfTheGame;

	PlayerOne->ApplyObjectDataToSaveData(save_data);

	if (boss_one != nullptr) {
		boss_one->ApplyObjectDataToSaveData(save_data);
	}

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		enemies[i]->ApplyObjectDataToSaveData(save_data[enemies[i]->GetName()]);
	}

	int doors_size = (int)doors.size();
	for (int i = 0; i < doors_size; i++) {
		doors[i]->ApplyObjectDataToSaveData(save_data[doors[i]->GetName()]);
	}

	Json::StreamWriterBuilder wbuilder;
	std::string document = Json::writeString(wbuilder, save_data);

	ofs << document;

	ofs.close();

	savedTextVisibleTimer->Start();
}

void SmashWorld::ImportSaveData(int save_slot) {
	if (save_slot != -1) {
		saveSlot = save_slot;
	}

	string file_name = "save" + to_string(saveSlot) + ".sav";
	Json::Value save_data;
	string rawData = "";
	string line;
	ifstream myfile(file_name, ios::binary);
	
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}
	
		myfile.close();
	} else {
		cout << "Unable to open file " << file_name << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, save_data);

	StageOfTheGame = save_data["WorldData"]["StageOfTheGame"].asString();

	PlayerOne->ApplySaveDataToObjectData(save_data);

	if (boss_one != nullptr) {
		boss_one->ApplySaveDataToObjectData(save_data);
	}

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		enemies[i]->ApplySaveDataToObjectData(save_data[enemies[i]->GetName()]);
	}

	int doors_size = (int)doors.size();
	for (int i = 0; i < doors_size; i++) {
		doors[i]->ApplySaveDataToObjectData(save_data[doors[i]->GetName()]);
	}

	myfile.close();
}

void ExternalResumeGame() {
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalExportSaveData() {
	Singleton<SmashWorld>::Get()->ExportSaveData();
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalImportSaveData() {
	Singleton<SmashWorld>::Get()->ImportSaveData();
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalExitGame() {
	Singleton<SmashWorld>::Get()->ExitGame();
}

void ExternalExitToMainMenu() {
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
	Singleton<SmashWorld>::Get()->ExitToMainMenu();
}

void ExternalOpenOptionsMenu() {
	Singleton<SmashWorld>::Get()->OpenOptionsMenu();
}

void ExternalCloseOptionsMenu() {
	Singleton<SmashWorld>::Get()->CloseOptionsMenu();
}

void ExternalSaveSettings() {
	Singleton<SmashWorld>::Get()->SaveSettings();
}

void ExternalEnableUsingArrowsForMovement() {
	Singleton<SmashWorld>::Get()->EnableUsingArrowsForMovement();
}

void ExternalDisableUsingArrowsForMovement() {
	Singleton<SmashWorld>::Get()->DisableUsingArrowsForMovement();
}

void SmashWorld::EnableUsingArrowsForMovement() {
	Singleton<Settings>::Get()->using_arrows_for_movement = true;
}

void SmashWorld::DisableUsingArrowsForMovement() {
	Singleton<Settings>::Get()->using_arrows_for_movement = false;
}

void ExternalEnableFullscreen() {
	Singleton<SmashWorld>::Get()->EnableFullscreen();
}

void ExternalDisableFullscreen() {
	Singleton<SmashWorld>::Get()->DisableFullscreen();
}

void SmashWorld::EnableFullscreen() {
	render_window->setActive(false);

	Singleton<Settings>::Get()->fullscreen = true;
	render_window->create(sf::VideoMode::getFullscreenModes()[0], "Project Boulder", sf::Style::Fullscreen);
	camera->viewport_dimensions = sf::Vector2f((float)render_window->getViewport(render_window->getView()).width, (float)render_window->getViewport(render_window->getView()).height);

	render_window->setActive(true);
}

void SmashWorld::DisableFullscreen() {
	render_window->setActive(false);

	Singleton<Settings>::Get()->fullscreen = false;
	render_window->create(sf::VideoMode::VideoMode((int)1280.0f, (int)720.0f), "Project Boulder");
	camera->viewport_dimensions = sf::Vector2f(1280.0f, 720.0f);

	render_window->setActive(true);
}

void SmashWorld::SaveSettings() {
	ofstream ofs("settings_data.txt", ios::binary | ios::out);

	Json::Value save_data;

	save_data["MusicVolume"] = Singleton<Settings>::Get()->music_volume;
	save_data["EffectsVolume"] = Singleton<Settings>::Get()->effects_volume;
	save_data["Fullscreen"] = Singleton<Settings>::Get()->fullscreen;
	save_data["UsingArrowsForMovement"] = Singleton<Settings>::Get()->using_arrows_for_movement;

	Json::StreamWriterBuilder wbuilder;
	std::string document = Json::writeString(wbuilder, save_data);

	ofs << document;

	ofs.close();
}

void SmashWorld::OpenOptionsMenu() {
	PauseMenu->Close();

	OptionsMenu->SetCurrentSliderValueByText("Music Volume", (int)Singleton<Settings>::Get()->music_volume);
	OptionsMenu->SetCurrentSliderValueByText("Effects Volume", (int)Singleton<Settings>::Get()->effects_volume);

	OptionsMenu->Open();
}

void SmashWorld::CloseOptionsMenu() {
	OptionsMenu->Close();

	UpdateEffectsSoundsThroughoutGame();

	PauseMenu->Open();
}

void SmashWorld::ExitGame() {
	render_window->close();
}

void SmashWorld::PlayerDied() {
	DeadMenu->Open();
}

void SmashWorld::CloseCurrentMenu() {
	bool was_a_menu_open = IsAMenuOpen();

	if (PauseMenu->IsOpen) {
		PauseMenu->Close();
		player_character_input->EatInputsForNumberOfFrames(1);

		if (enemyNearby) {
			musicManager->FadeToSong(combatMusicFileName);
		} else if (PlayerOne->IsSkateboarding) {
			musicManager->FadeToSong(travelingMusicFileName);
		} else {
			musicManager->FadeToSong(downTimeMusicFileName);
		}
	} else if (DeadMenu->IsOpen) {
		DeadMenu->Close();
		player_character_input->EatInputsForNumberOfFrames(1);
	} else if (OptionsMenu->IsOpen) {
		CloseOptionsMenu();
		player_character_input->EatInputsForNumberOfFrames(1);
	} 
}

void SmashWorld::Setup() {
	exit_to_main_menu = false;
	unit_type_player_is_talking_to = "";
	boss_one_fight_started = false;
	
	gravity = new b2Vec2(0.0f, 30.0f);
	world = new b2World(*gravity);

	//ParseWorld("Maps\\TestRails");
	ParseWorld("Maps\\PreAlphaMap");
	ParsePlayerBestiary("Units\\PlayerBestiary.txt");
	ParseBestiaries();
	ParseDialogue("SirensStory.txt");

	BuildWorld();

	velocityIterations = 6;
	positionIterations = 2;

	test_drawable = new Drawable(render_window, camera);
	test_drawable->SetFlags(b2Draw::e_shapeBit);
	
	world->SetDebugDraw(test_drawable);

	world->SetContactListener(&myContactListenerInstance);

	dialogue_text = sf::Text("", ringbearer_font, 45);
	dialogue_text.setPosition(100.0f, camera->viewport_dimensions.y - 200.0f);
	dialogue_text.setFillColor(sf::Color::White);
	dialogue_text.setOutlineThickness(3.0f);
	dialogue_text.setOutlineColor(sf::Color::Black);

	savedText = sf::Text("Game Saved", ringbearer_font, 35);
	savedText.setPosition(camera->viewport_dimensions.x / 2.0f - 100.0f, camera->viewport_dimensions.y - 60.0f);
	savedText.setFillColor(sf::Color::White);
	savedText.setOutlineThickness(3.0f);
	savedText.setOutlineColor(sf::Color::Black);

	savedTextVisibleTimer = new StatusTimer(240);

	CurrentDialogueLine = nullptr;
	RootDialogueLine = new DialogueLine("1", nullptr, jsonDialogueData);

	PauseMenu = new Menu(render_window, camera->viewport_dimensions, "Images/parallax_background_old.jpg");
	PauseMenu->AddItem("Resume Game", &ExternalResumeGame);
	PauseMenu->AddItem("Save Game", &ExternalExportSaveData);
	PauseMenu->AddItem("Load Game", &ExternalImportSaveData);
	PauseMenu->AddItem("Options", &ExternalOpenOptionsMenu);
	PauseMenu->AddItem("Go To Main Menu", &ExternalExitToMainMenu);
	PauseMenu->AddItem("Exit Game", &ExternalExitGame);

	DeadMenu = new Menu(render_window, camera->viewport_dimensions, "Images/parallax_background.jpg");
	DeadMenu->AddItem("Load Game", &ExternalImportSaveData);
	DeadMenu->AddItem("Go To Main Menu", &ExternalExitToMainMenu);
	DeadMenu->AddItem("Exit Game", &ExternalExitGame);

	OptionsMenu = new Menu(render_window, camera->viewport_dimensions, "Images/parallax_background0.jpg");
	OptionsMenu->AddItem("Music Volume", (int)Singleton<Settings>::Get()->music_volume, 100);
	OptionsMenu->AddItem("Effects Volume", (int)Singleton<Settings>::Get()->effects_volume, 100);
	OptionsMenu->AddItem("Fullscreen", Singleton<Settings>::Get()->fullscreen, &ExternalEnableFullscreen, &ExternalDisableFullscreen);
	OptionsMenu->AddItem("Use Arrows For Movement", Singleton<Settings>::Get()->using_arrows_for_movement, &ExternalEnableUsingArrowsForMovement, &ExternalDisableUsingArrowsForMovement);
	OptionsMenu->AddItem("Save Settings", &ExternalSaveSettings);
	OptionsMenu->AddItem("Back", &ExternalCloseOptionsMenu);

	//CharScreen = new CharacterScreen(PlayerOne, render_window, camera);

	ButtonsTexture = Singleton<AssetManager>().Get()->GetTexture("Images/Buttons.png");
	ButtonsSprite = new sf::Sprite(*ButtonsTexture);
	ButtonsSprite->setPosition(5.0f, camera->viewport_dimensions.y - 130.0f);
	
	UpdateEffectsSoundsThroughoutGame();

	if (!audioCommentary.openFromFile("Sound/AudioCommentary02062018.wav"))
		return;
	audioCommentary.setVolume(50.0f * (Singleton<Settings>::Get()->music_volume / 100.0f));
	audioCommentary.setLoop(false);

	currentZone = 1;

	past_setup = true;
}

void SmashWorld::UpdateLoadingScreen() {
	render_window->setActive(true);

	float loadingRectAngle = 0.0f;
	int numberOfDots = 0;

	sf::RectangleShape rect = sf::RectangleShape(sf::Vector2f(100.0f, 100.0f));
	rect.setPosition(camera->viewport_dimensions.x / 2.0f, camera->viewport_dimensions.y / 2.0f - 50.0f);
	rect.setOrigin(50.0f, 50.0f);

	sf::Text loading_text = sf::Text("Loading", ringbearer_font, 45);
	loading_text.setPosition(camera->viewport_dimensions.x / 2.0f - 90.0f, camera->viewport_dimensions.y / 2.0f + 50.0f);
	loading_text.setFillColor(sf::Color::Black);
	loading_text.setOutlineThickness(3.0f);
	loading_text.setOutlineColor(sf::Color::White);

	while (!past_setup) {
		render_window->clear();

		loadingRectAngle = loadingRectAngle + 0.1f;
		if (loadingRectAngle >= 360.0f) {
			loadingRectAngle = 0.0f;

			numberOfDots++;

			if (numberOfDots > 3) {
				numberOfDots = 0;
			}

			if (numberOfDots == 0) {
				loading_text.setString("Loading");
			} else if (numberOfDots == 1) {
				loading_text.setString("Loading.");
			} else if (numberOfDots == 2) {
				loading_text.setString("Loading..");
			} else if (numberOfDots == 3) {
				loading_text.setString("Loading...");
			}
		}

		rect.setRotation(loadingRectAngle);
		render_window->draw(rect);
		render_window->draw(loading_text);

		render_window->display();
	}

	render_window->setActive(false);
}

void SmashWorld::UpdateVideo() {
	render_window->setActive(true);

#ifdef _DEBUG
	//while (intro_cutscene.getStatus() == sfe::Playing) {
	//	render_window->clear();
	//
	//	//player_menu_input->Update();
	//
	//	intro_cutscene.update();
	//	render_window->draw(intro_cutscene);
	//
	//	render_window->display();
	//}
#else
#endif

}

bool SmashWorld::Update(sf::Int64 curr_frame, sf::Int64 frame_delta) {
	if (!render_window->hasFocus()) {
		return false;
	}

	UpdateMusic();
	musicManager->Update();

	render_window->clear();

#ifdef _DEBUG
	//if (intro_cutscene.getStatus() == sfe::Playing) {
	//	//player_menu_input->Update();
	//
	//	intro_cutscene.update();
	//	render_window->draw(intro_cutscene);
	//} else 
#else
#endif
	if (PauseMenu->IsOpen) {
		player_menu_input->Update();
		PauseMenu->Draw(curr_frame);
	} else if (DeadMenu->IsOpen) {
		player_menu_input->Update();
		DeadMenu->Draw(curr_frame);
	} else if (OptionsMenu->IsOpen) {
		player_menu_input->Update();
		OptionsMenu->Draw(curr_frame);
	//} else if (CharScreen->IsOpen) {
	//	player_menu_input->Update();
	//	CharScreen->Draw(camera->viewport_dimensions);
	} else {
		current_frame = curr_frame;

		world->Step(timeStep, velocityIterations, positionIterations);

		camera->Update(current_frame, frame_delta, PlayerOne->GetBody()->GetPosition().x, PlayerOne->GetBody()->GetPosition().y);

		if (unit_type_player_is_talking_to != "") {
			b2Vec2 body_lin_vel = PlayerOne->GetBody()->GetLinearVelocity();
			PlayerOne->GetBody()->SetLinearVelocity(b2Vec2(body_lin_vel.x * 0.9f, body_lin_vel.y));
			player_menu_input->Update();
		} else {
			player_character_input->Update();
		}

		for (int i = 0; i < zones[currentZone - 1]->parallax_background_sprites_size; i++) {
			zones[currentZone - 1]->parallax_background_viewport_position = sf::Vector2f(-(zones[currentZone - 1]->x + camera->viewport_position.x * (i * 3.0f)), -(zones[currentZone - 1]->y + camera->viewport_position.y * (i * 3.0f)));
			zones[currentZone - 1]->parallax_background_sprites[i]->setPosition(zones[currentZone - 1]->parallax_background_viewport_position);

			//shader.setUniform("timer", (float)curr_frame);
			//shader.setUniform("x_magnitude", 0.0f);// 0.035f);
			//shader.setUniform("y_magnitude", 0.0f);// 0.015f);

			//lighting_shader.setUniform("lightPosition", sf::Glsl::Vec2((PlayerOne->GetBody()->GetPosition().x - camera->viewport_position.x) * 40.0f, (PlayerOne->GetBody()->GetPosition().y - camera->viewport_position.y) * 40.0f));
			render_window->draw(*zones[currentZone - 1]->parallax_background_sprites[i]);// , &shader);
		}

		world->DrawDebugData();

		PlayerOne->Update(current_frame, frame_delta);
		PlayerOne->Draw(camera->viewport_position);

		sf::Vector2f player_screen_pos = PlayerOne->GetScreenPosition();

		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			if (doors[i]->IfShouldUpdate(player_screen_pos, camera->viewport_dimensions)) {
				doors[i]->Update(current_frame, frame_delta);
				doors[i]->Draw(camera->viewport_position);
			}
		}

		if (boss_one_fight_started) {
			if (boss_one->IfShouldUpdate(player_screen_pos, camera->viewport_dimensions)) {
				boss_one->Update(current_frame, frame_delta);
				boss_one->Draw(camera->viewport_position);
			}
		}

		int box_2d_rigid_bodies_size = (int)box2dRigidBodies.size();
		for (int i = 0; i < box_2d_rigid_bodies_size; i++) {
			if (box2dRigidBodies[i]->IfShouldUpdate(player_screen_pos, camera->viewport_dimensions)) {
				box2dRigidBodies[i]->Update(current_frame, frame_delta);
				box2dRigidBodies[i]->Draw(camera->viewport_position);
			}
		}

		int enemies_size = (int)enemies.size();
		for (int i = 0; i < enemies_size; i++) {
			if (enemies[i]->IfShouldUpdate(player_screen_pos, camera->viewport_dimensions)) {
				enemies[i]->Update(current_frame, frame_delta);
				enemies[i]->Draw(camera->viewport_position);
			}
		}

		bool was_enemy_nearby = enemyNearby;
		enemyNearby = false;
		for (int i = 0; i < enemies_size; i++) {
			if (enemies[i]->IsNearPlayer(player_screen_pos, camera->viewport_dimensions)) {
				enemyNearby = true;
				break;
			}
		}

		if (was_enemy_nearby && !enemyNearby) {
			musicManager->FadeToSong(downTimeMusicFileName);
		}

		int platform_sprites_size = (int)imageSprites.size();
		for (int i = 0; i < platform_sprites_size; i++) {
			imageSprites[i]->setPosition(sf::Vector2f((imageXs[i] - camera->viewport_position.x) * 40.0f + 0.0f, (imageYs[i] - camera->viewport_position.y) * 40.0f + 0.0f));

			float delta_x = abs(player_screen_pos.x - imageSprites[i]->getPosition().x);
			float delta_y = abs(player_screen_pos.y - imageSprites[i]->getPosition().y);

			if (delta_x > camera->viewport_dimensions.x * 40.00f) {
				continue;
			}

			if (delta_y > camera->viewport_dimensions.y * 40.00f) {
				continue;
			}

			render_window->draw(*imageSprites[i]);
		}

		render_window->draw(*ButtonsSprite);

		HandleDisplayingNextDialogueCharacter();
		render_window->draw(dialogue_text);
	}

	if (savedTextVisibleTimer->IsActive()) {
		render_window->draw(savedText);
	}

	render_window->display();

	return goToCredits;
}

void SmashWorld::ScreenShake(float magnitude) {
	camera->ScreenShake(magnitude);
}

void SmashWorld::ParseWorld(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	jsonWorldData.clear();
	reader.parse(rawData, jsonWorldData);
}

void SmashWorld::ParsePlayerBestiary(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	jsonPlayerData.clear();
	reader.parse(rawData, jsonPlayerData);
}

void SmashWorld::ParseDialogue(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	}
	else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	jsonDialogueData.clear();
	reader.parse(rawData, jsonDialogueData);
}

void SmashWorld::BuildWorld() {
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float vel_x = 0.0f;
	float vel_y = 0.0f;
	bool pass_through = false;
	string tied_art_image_file_name = "";
	std::string::size_type sz;

	float scalingRatio = 10.0f;

	Json::Value player_data = jsonWorldData["player"];
	Json::Value rectangles_data = jsonWorldData["rectangles"];
	Json::Value triangles_data = jsonWorldData["triangles"];
	Json::Value rails_data = jsonWorldData["rails"];
	Json::Value doors_data = jsonWorldData["doors"];
	Json::Value triggers_data = jsonWorldData["triggers"];
	Json::Value units_data = jsonWorldData["units"];
	Json::Value images_data = jsonWorldData["images"];
	Json::Value zones_data = jsonWorldData["zones"];

	x = std::stof(player_data["x"].asString(), &sz) / scalingRatio;
	y = std::stof(player_data["y"].asString(), &sz) / scalingRatio;
	width = std::stof(player_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
	height = std::stof(player_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

	x += width;
	y += height;

	PlayerOne = new SmashCharacter(0, jsonPlayerData, render_window, sf::Vector2f(x, y), sf::Vector2f(0.3f, 1.0f));
	PlayerOne->SetName(jsonWorldData["player"]["name"].asString());
	player_character_input = new InputHandler(PlayerOne);

	sf::Vector2f camera_position = camera->viewport_position;
	camera_position.x += (PlayerOne->GetBody()->GetPosition().x - camera->viewport_dimensions.x / 2.0f / 40.0f - camera_position.x);
	camera_position.y += (PlayerOne->GetBody()->GetPosition().y - camera->viewport_dimensions.y / 2.0f / 40.0f - camera_position.y);
	camera->viewport_position = camera_position;

	box2dRigidBodies.clear();
	Json::Value rectangle_data;
	int rectangles_data_size = (int)rectangles_data.size();
	for (int i = 0; i < rectangles_data_size; i++) {
		rectangle_data = rectangles_data[i];

		x = std::stof(rectangle_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(rectangle_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(rectangle_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(rectangle_data["height"].asString(), &sz) / (scalingRatio * 2.0f);
		vel_x = rectangle_data["vel_x"].asFloat();
		vel_y = rectangle_data["vel_y"].asFloat();
		vel_y = rectangle_data["vel_y"].asFloat();
		pass_through = rectangle_data["pass_through"].asBool();
		tied_art_image_file_name = rectangle_data["TiedArtImageFileName"].asString();

		x += width;
		y += height;

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, rectangle_data["name"].asString(), sf::Vector2f(x, y), sf::Vector2f(width, height)));
		box2dRigidBodies[i]->SetPassThrough(pass_through);
		box2dRigidBodies[i]->TiedArtImageFileName = tied_art_image_file_name;

		if (vel_x != 0.0f || vel_y != 0.0f) {
			box2dRigidBodies[i]->ConvertToMovingPlatform(vel_x, vel_y);
		}
	}

	Json::Value triangle_data;
	int triangles_data_size = (int)triangles_data.size();
	for (int i = 0; i < triangles_data_size; i++) {
		triangle_data = triangles_data[i];

		std::vector<string> vects;
		vects.push_back(triangle_data["points"][0].asString());
		vects.push_back(triangle_data["points"][1].asString());
		vects.push_back(triangle_data["points"][2].asString());

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, triangle_data["name"].asString(), vects));
	}

	Json::Value rail_data;
	int rails_data_size = (int)rails_data.size();
	for (int i = 0; i < rails_data_size; i++) {
		rail_data = rails_data[i];

		float x1 = rail_data["x"].asFloat() / (scalingRatio * 2.0f);
		float y1 = rail_data["y"].asFloat() / (scalingRatio * 2.0f);
		float x2 = rail_data["x_prime"].asFloat() / (scalingRatio * 2.0f);
		float y2 = rail_data["y_prime"].asFloat() / (scalingRatio * 2.0f);
		x2 += x2 - x1;
		y2 += y2 - y1;
		
		cout << x1 << " " << y1 << " " << x2 << " " << y2 << "\n";

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, rail_data["name"].asString(), x1, y1, x2, y2));
		box2dRigidBodies.back()->IsGrindRail = true;
	}

	doors.clear();
	Json::Value door_data;
	int doors_data_size = (int)doors_data.size();
	for (int i = 0; i < doors_data_size; i++) {
		door_data = doors_data[i];

		x = std::stof(door_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(door_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(door_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(door_data["height"].asString(), &sz) / (scalingRatio * 2.0f);
		tied_art_image_file_name = door_data["TiedArtImageFileName"].asString();

		x += width;
		y += height;

		doors.push_back(new Door(render_window, door_data["name"].asString(), sf::Vector2f(x, y), sf::Vector2f(width, height)));
		doors[i]->AddActivator(jsonWorldData["doors"][i]["activator"].asString());
		doors[i]->TiedArtImageFileName = tied_art_image_file_name;
	}

	triggers.clear();
	Json::Value trigger_data;
	int triggers_data_size = (int)triggers_data.size();
	for (int i = 0; i < triggers_data_size; i++) {
		trigger_data = triggers_data[i];

		x = std::stof(trigger_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(trigger_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(trigger_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(trigger_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		triggers.push_back(new Trigger(trigger_data["name"].asString(), render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
		triggers[i]->AddActivaty(trigger_data["activity"].asString());
	}

	enemies.clear();
	Json::Value unit_data;
	int units_data_size = (int)units_data.size();
	for (int i = 0; i < units_data_size; i++) {
		unit_data = units_data[i];

		x = std::stof(unit_data["LevelLocationX"].asString(), &sz) / scalingRatio;
		y = std::stof(unit_data["LevelLocationY"].asString(), &sz) / scalingRatio;
		width = std::stof(unit_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(unit_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		Json::Value thisBestiary;
		for (int j = 0; j < (int)jsonBestiariesData.size(); j++) {
			if (jsonBestiariesData[j]["BestiaryName"] == unit_data["BestiaryName"].asString()) {
				thisBestiary = jsonBestiariesData[j];
			}
		}

		//string test0 = jsonBestiariesData[0]["BestiaryName"].asString();
		//string test1 = thisBestiary["DictOfUnits"]["Gelly"]["IdleAnimations"][0]["FilePath"].asString();

		if (Contains(unit_data["UnitType"].asString(), "BossOne")) {
			boss_one = new BossOne(unit_data, thisBestiary,
				render_window, sf::Vector2f(x, y), sf::Vector2f(width, height));
			boss_one->AddActivaty(unit_data["activity"].asString());
		} else {
			enemies.push_back(new BoulderCreature(unit_data, thisBestiary,
				render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
			if (!unit_data["activity"].isNull()) {
				string test = unit_data["activity"].asString();
				enemies[(int)enemies.size() - 1]->AddActivaty(test);
			}
		}
	}

	imageTextures.clear();
	imageSprites.clear();
	imageXs.clear();
	imageYs.clear();
	imageNames.clear();
	sf::Sprite* current_sprite;
	bool is_standalone = true;
	int standalone_images = 0;
	Json::Value image_data;
	int images_data_size = (int)images_data.size();
	for (int i = 0; i < images_data_size; i++) {
		image_data = images_data[i];

		string file_path_and_name = image_data["FilePath"].asString();
		size_t findResult = file_path_and_name.find("Images\\");
		string relativeFilePath = file_path_and_name.substr(findResult);

		is_standalone = image_data["IsStandalone"].asBool();

		x = std::stof(image_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(image_data["y"].asString(), &sz) / scalingRatio;

		imageTextures.push_back(Singleton<AssetManager>().Get()->GetTexture(relativeFilePath));
		current_sprite = new sf::Sprite(*imageTextures[i]);

		if (is_standalone) {
			imageSprites.push_back(current_sprite);
			imageSprites[standalone_images]->setScale(0.666667f, 0.68f);
			imageXs.push_back(x);
			imageYs.push_back(y);
			imageNames.push_back(image_data["name"].asString());

			standalone_images++;
		} else {
			for (int rect = 0; rect < rectangles_data_size; rect++) {
				if (Utilities::Contains(box2dRigidBodies[rect]->TiedArtImageFileName, relativeFilePath)) {
					box2dRigidBodies[rect]->LoadArtImage(relativeFilePath);
				}
			}

			for (int door = 0; door < doors_data_size; door++) {
				if (Utilities::Contains(doors[door]->TiedArtImageFileName, relativeFilePath)) {
					doors[door]->LoadArtImage(relativeFilePath);
				}
			}
		}
	}

	int zones_data_size = (int)zones_data.size();
	for (int z = 0; z < zones_data_size; z++) {
		zones.push_back(new Zone());
		Json::Value parallaxing_backgrounds_data = jsonWorldData["zones"][z]["ParallaxingBackgrounds"];

		zones[z]->parallax_background_textures.clear();
		zones[z]->parallax_background_sprites.clear();
		Json::Value parallaxing_background_data;
		int parallaxing_backgrounds_data_size = (int)parallaxing_backgrounds_data.size();
		zones[z]->parallax_background_sprites_size = parallaxing_backgrounds_data_size;
		for (int i = 0; i < parallaxing_backgrounds_data_size; i++) {
			parallaxing_background_data = parallaxing_backgrounds_data[i];

			string file_path_and_name = parallaxing_background_data["FilePath"].asString();
			size_t findResult = file_path_and_name.find("Images\\");
			string relativeFilePath = file_path_and_name.substr(findResult);

			x = std::stof(parallaxing_background_data["x"].asString(), &sz) / scalingRatio;
			y = std::stof(parallaxing_background_data["y"].asString(), &sz) / scalingRatio;

			zones[z]->x = x;
			zones[z]->y = y;
			zones[z]->parallax_background_textures.insert(zones[z]->parallax_background_textures.begin(), Singleton<AssetManager>().Get()->GetTexture(relativeFilePath));//.loadFromFile("Images/parallax_background.jpg");
			zones[z]->parallax_background_sprites.insert(zones[z]->parallax_background_sprites.begin(), new sf::Sprite(*zones[z]->parallax_background_textures[0]));
			zones[z]->parallax_background_sprites[0]->setPosition(0.0f, 0.0f);
			zones[z]->parallax_background_sprites[0]->setScale(1.0f + 0.2f * (parallaxing_backgrounds_data_size - i), 1.0f + 0.2f * (parallaxing_backgrounds_data_size - i));

			//parallax_background_texture.loadFromFile("Images/parallax_background.jpg");
			//parallax_background_sprite = sf::Sprite(parallax_background_texture);
			//parallax_background_sprite.setPosition(0.0f, 0.0f);
		}
	}
}

void SmashWorld::ParseBestiaries() {
	jsonBestiariesData.clear();
	int bestiary_file_paths_size = (int)jsonWorldData["bestiaryFilePaths"].size();
	for (int i = 0; i < bestiary_file_paths_size; i++)
	{
		ParseBestiary(jsonWorldData["bestiaryFilePaths"][i].asString());
	}
}

void SmashWorld::ParseBestiary(string file_path) {
	string relativeFilePath = "";

	if (Utilities::Contains(file_path, "Units\\")) {
		size_t findResult = file_path.find("Units\\");
		relativeFilePath = file_path.substr(findResult);
	} else if (Utilities::Contains(file_path, "Destructibles\\")) {
		size_t findResult = file_path.find("Destructibles\\");
		relativeFilePath = file_path.substr(findResult);
	}

	string rawData = "";
	Json::Value jsonData = "";
	string line;
	ifstream myfile(relativeFilePath);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << relativeFilePath << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonData);

	jsonBestiariesData.push_back(jsonData);
}

void SmashWorld::SetDialogueText(string new_text) {
	dialogue_text.setString(new_text);
}

void SmashWorld::StartDialogue(string unit_type, string unit_name) {
	unit_type_player_is_talking_to = unit_type;

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		if (enemies[i]->GetName() == unit_name) {
			UnitBeingTalkedTo = enemies[i];
			break;
		}
	}

	ProgressDialogueText();
}

void SmashWorld::ProgressDialogueText() {
	if (dialogueStringThatIsBeingSaid != "" && numberOfDialogueCharactersBeingDisplayed != (int)dialogueStringThatIsBeingSaid.length() - 1) {
		for (int i = numberOfDialogueCharactersBeingDisplayed; i < (int)dialogueStringThatIsBeingSaid.length(); i++) {
			if (doesPlayerHaveHeadphonesOn) {
				string newCharacter = dialogueStringThatIsBeingSaid.substr(i, 1);

				if (newCharacter == " " || newCharacter == "\n") {
					dialogueStringThatIsAppearing.append(newCharacter);
				} else {
					dialogueStringThatIsAppearing.append("X");
				}
			} else {
				dialogueStringThatIsAppearing.append(dialogueStringThatIsBeingSaid.substr(i, 1));
			}
		}

		dialogue_text.setString(dialogueStringThatIsAppearing);

		numberOfDialogueCharactersBeingDisplayed = (int)dialogueStringThatIsBeingSaid.length() - 1;
	} else {
		if (CurrentDialogueLine == nullptr) {
			CurrentDialogueLine = RootDialogueLine->GetFirstRelevantDialogueLine(StageOfTheGame);
		} else if (CurrentDialogueLine->GetNextDialogueLine(StageOfTheGame) != nullptr) {
			CurrentDialogueLine = CurrentDialogueLine->GetNextDialogueLine(StageOfTheGame);
		} else {
			CloseDialogue();
		}

		player_menu_input->EatInputsForNumberOfFrames(1);

		if (CurrentDialogueLine != nullptr) {
			numberOfDialogueCharactersBeingDisplayed = 0;
			dialogueStringThatIsAppearing = "";
			dialogueStringThatIsBeingSaid = CurrentDialogueLine->Line;

			if (CurrentDialogueLine->ExecuteActionsWhenLineIsHit) {
				UnitBeingTalkedTo->ExecuteActions();
			}
		}
	}
}

void SmashWorld::HandleDisplayingNextDialogueCharacter() {
	if (unit_type_player_is_talking_to != "") {

		if (frameOfLastDialogueCharacterAppeared + framesBetweenDialogueCharactersAppearing < current_frame &&
			dialogueStringThatIsAppearing.length() != dialogueStringThatIsBeingSaid.length()) {
			frameOfLastDialogueCharacterAppeared = current_frame;

			if (doesPlayerHaveHeadphonesOn) {
				string newCharacter = dialogueStringThatIsBeingSaid.substr(numberOfDialogueCharactersBeingDisplayed, 1);

				if (newCharacter == " " || newCharacter == "\n") {
					dialogueStringThatIsAppearing.append(dialogueStringThatIsBeingSaid.substr(numberOfDialogueCharactersBeingDisplayed, 1));
				} else {
					dialogueStringThatIsAppearing.append("X");
				}
			} else {
				dialogueStringThatIsAppearing.append(dialogueStringThatIsBeingSaid.substr(numberOfDialogueCharactersBeingDisplayed, 1));
			}

			dialogue_text.setString(dialogueStringThatIsAppearing.substr(0, numberOfDialogueCharactersBeingDisplayed));

			numberOfDialogueCharactersBeingDisplayed++;

			if (numberOfDialogueCharactersBeingDisplayed == (int)dialogueStringThatIsBeingSaid.length()) {
				numberOfDialogueCharactersBeingDisplayed = (int)dialogueStringThatIsBeingSaid.length() - 1;
			}
		}
	}
}

void SmashWorld::ExecuteAction(string action_call) {
	std::vector<string> vstrings = Utilities::Split(action_call, '(');

	string call = vstrings[0];
	string argumentString = vstrings[1].substr(0, vstrings[1].size() - 1);

	std::vector<string> arguments = Utilities::Split(argumentString, ',');

	if (Utilities::Contains(call, "LockDoor")) {
		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			if (doors[i]->Name == arguments[0]) {
				doors[i]->SetLocked(true);
			}
		}
	} else if (Utilities::Contains(call, "UnlockDoor")) {
		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			if (doors[i]->Name == arguments[0]) {
				doors[i]->SetLocked(false);
			}
		}
	} else if (Utilities::Contains(call, "OpenDoor")) {
		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			if (doors[i]->Name == arguments[0]) {
				doors[i]->OpenDoor();
			}
		}
	} else if (Utilities::Contains(call, "CloseDoor")) {
		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			if (doors[i]->Name == arguments[0]) {
				doors[i]->CloseDoor();
			}
		}
	} else if (Utilities::Contains(call, "ChangeStage")) {
		StageOfTheGame = arguments[0];
	} else if (Utilities::Contains(call, "Aggro")) {
		if (boss_one->GetName() == arguments[0]) {
			boss_one->Aggro((BoulderCreature*)PlayerOne);
			boss_one_fight_started = true;
		}
	} else if (Utilities::Contains(call, "EndGame")) {
		goToCredits = true;
	} else if (Utilities::Contains(call, "ForcedRecall")) {
		PlayerOne->ForcedRecall();
	//} else if (Utilities::Contains(call, "PickUpRune")) {
	//	PlayerOne->PickUpRune(arguments[0]);
	} else if (Utilities::Contains(call, "RemoveImage")) {
		int image_names_size = (int)imageNames.size();
		for (int i = 0; i < image_names_size; i++) {
			if (imageNames[i] == arguments[0]) {
				imageNames.erase(imageNames.begin() + i);
				imageTextures.erase(imageTextures.begin() + i);
				imageSprites.erase(imageSprites.begin() + i);
				imageXs.erase(imageXs.begin() + i);
				imageYs.erase(imageYs.begin() + i);

				break;
			}
		}
	} else if (Utilities::Contains(call, "ActivateElevator")) {
		std::string::size_type sz;
		int platforms_size = (int)box2dRigidBodies.size();
		for (int i = 0; i < platforms_size; i++) {
			if (box2dRigidBodies[i]->Name == arguments[0]) {
				float vel_x = std::stof(arguments[1], &sz);
				float vel_y = std::stof(arguments[2], &sz);

				box2dRigidBodies[i]->StartMovingElevator(vel_x, vel_y);

				break;
			}
		}
	} else if (Utilities::Contains(call, "SwitchToZone")) {
		if (to_string(currentZone) != arguments[0])
		{
			std::string::size_type sz;
			int new_zone = std::stoi(arguments[0], &sz);
			currentZone = new_zone;
		}
	} else if (Utilities::Contains(call, "HealPlayer")) {
		std::string::size_type sz;
		int randChance = std::stoi(arguments[0], &sz);
		int randOutOf = std::stoi(arguments[1], &sz);
		int healingValue = std::stoi(arguments[2], &sz);

		if (randChance <= rand() % randOutOf + 1) {
			PlayerOne->ReceiveHeal(healingValue);
		}
	}
}

void SmashWorld::ExitToMainMenu() {
	exit_to_main_menu = true;
}

bool SmashWorld::ShouldExitToMainMenu() {
	return exit_to_main_menu;
}

string SmashWorld::GetCurrentPointInGame() {
	return "Intro";
}

void SmashWorld::HandleLeftStickInput(float horizontal, float vertical) {
	if (IsAMenuOpen()) {
		if (vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			if (PauseMenu->IsOpen) {
				PauseMenu->MoveCursorDown();
			} else if (DeadMenu->IsOpen) {
				DeadMenu->MoveCursorDown();
			} else if (OptionsMenu->IsOpen) {
				OptionsMenu->MoveCursorDown();
			}
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			if (PauseMenu->IsOpen) {
				PauseMenu->MoveCursorUp();
			} else if (DeadMenu->IsOpen) {
				DeadMenu->MoveCursorUp();
			} else if (OptionsMenu->IsOpen) {
				OptionsMenu->MoveCursorUp();
			}
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (vertical >= -90.0f && vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}

		int return_value = 0;
		bool apply_return_value = false;

		if (horizontal > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			return_value = OptionsMenu->MoveCursorRight();
			apply_return_value = true;
		} else if (horizontal < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			return_value = OptionsMenu->MoveCursorLeft();
			apply_return_value = true;
		}

		if (apply_return_value) {
			if (OptionsMenu->GetCurrentSelectionText() == "Music Volume") {
				Singleton<Settings>::Get()->music_volume = (float)return_value;
			} else if (OptionsMenu->GetCurrentSelectionText() == "Effects Volume") {
				Singleton<Settings>::Get()->effects_volume = (float)return_value;
			}
		}
	}
}

void SmashWorld::HandleRightStickInput(float horizontal, float vertical) {
}

void SmashWorld::HandleButtonBPress() {
	if ((PauseMenu->IsOpen || OptionsMenu->IsOpen) && PlayerOne->hit_points > 0) {
		CloseCurrentMenu();
	} else if (unit_type_player_is_talking_to != "") {
		CloseDialogue();
	}
}

void SmashWorld::CloseDialogue() {
	if (unit_type_player_is_talking_to != "") {
		unit_type_player_is_talking_to = "";
		CurrentDialogueLine = nullptr;
		dialogue_text.setString("");
		dialogueStringThatIsBeingSaid = "";
	}
}

bool SmashWorld::IsAMenuOpen() {
	return PauseMenu->IsOpen || DeadMenu->IsOpen || OptionsMenu->IsOpen;
}

void SmashWorld::UpdateEffectsSoundsThroughoutGame() {
	float new_effects_volume = (float)OptionsMenu->GetCurrentSliderValueByText("Effects Volume");

	PlayerOne->UpdateEffectsVolumes(new_effects_volume);

	// Doors will eventually have sounds.
	//int doors_size = (int)doors.size();
	//for (int i = 0; i < doors_size; i++) {
	//	doors[i]->UpdateEffectsVolumes(new_effects_volume);
	//}

	if (boss_one_fight_started) {
		boss_one->UpdateEffectsVolumes(new_effects_volume);
	}

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		enemies[i]->UpdateEffectsVolumes(new_effects_volume);
	}
}

void SmashWorld::HandleButtonBRelease() {
}

void SmashWorld::HandleButtonXPress() {
	if (unit_type_player_is_talking_to != "") {
		ProgressDialogueText();
	}
}

void SmashWorld::HandleButtonXRelease() {
}

void SmashWorld::HandleButtonAPress() {
#ifdef _DEBUG
	//if (intro_cutscene.getStatus() == sfe::Playing) {
	//	intro_cutscene.stop();
	//} else 
#else
#endif
	if (PauseMenu->IsOpen) {
		PauseMenu->ExecuteCurrentSelection();
	} else if (DeadMenu->IsOpen) {
		DeadMenu->ExecuteCurrentSelection();
	} else if (OptionsMenu->IsOpen) {
		OptionsMenu->ExecuteCurrentSelection();
	}
}

void SmashWorld::HandleButtonARelease() {
}

void SmashWorld::HandleButtonYPress() {
}

void SmashWorld::HandleButtonYRelease() {
}

void SmashWorld::HandleButtonStartPress() {
	if (PauseMenu->IsOpen) {
		PauseMenu->ExecuteCurrentSelection();
	} else if (DeadMenu->IsOpen) {
		DeadMenu->ExecuteCurrentSelection();
	} else if (OptionsMenu->IsOpen) {
		OptionsMenu->ExecuteCurrentSelection();
	}
}

void SmashWorld::HandleButtonStartRelease() {
}

void SmashWorld::HandleButtonSelectPress() {
#ifdef _DEBUG
	//if (intro_cutscene.getStatus() == sfe::Playing) {
	//	intro_cutscene.stop();
	//}
	//else
#else
#endif
	if (past_setup && !DeadMenu->IsOpen && !OptionsMenu->IsOpen) {
		PauseMenu->Open();

		musicManager->FadeToSong(menuMusicFileName);
	} else {
		CloseCurrentMenu();
	}
}

void SmashWorld::HandleButtonSelectRelease() {
}

void SmashWorld::StartAudioCommentary() {
	audioCommentary.setVolume(Singleton<Settings>::Get()->music_volume);
	audioCommentary.play();
}

void SmashWorld::EnemyDied(int experience_points) {
}

void SmashWorld::HandleDpadRightPress() {
}

void SmashWorld::HandleDpadRightRelease() {
}

void SmashWorld::HandleDpadLeftPress() {
}

void SmashWorld::HandleDpadLeftRelease() {
}

void SmashWorld::HandleDpadUpPress() {
}

void SmashWorld::HandleDpadUpRelease() {
}

void SmashWorld::HandleDpadDownPress() {
	TakeHeadphonesOff();
}

void SmashWorld::HandleDpadDownRelease() {
	PutHeadphonesOn();
}

void SmashWorld::UpdateMusic() {
	if (timeSkateBoardWasPutAway + timeTravelingMusicPlaysForAfterSkateboardIsPutAway < current_frame && !PlayerOne->IsSkateboarding && !enemyNearby && playingSkateboardingMusic) {
		playingSkateboardingMusic = false;
		musicManager->FadeToSong(downTimeMusicFileName);
	}
}

void SmashWorld::StartCombatMusic() {
	musicManager->FadeToSong(combatMusicFileName, 20);
}

void SmashWorld::PutAwaySkateBoard() {
	timeSkateBoardWasPutAway = current_frame;
}

void SmashWorld::StartedUsingSkateBoard() {
	playingSkateboardingMusic = true;
	musicManager->FadeToSong(travelingMusicFileName);
}

void SmashWorld::TakeHeadphonesOff() {
	doesPlayerHaveHeadphonesOn = false;
}

void SmashWorld::PutHeadphonesOn() {
	doesPlayerHaveHeadphonesOn = true;
}