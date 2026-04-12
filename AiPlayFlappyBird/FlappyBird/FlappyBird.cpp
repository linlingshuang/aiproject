﻿#include <SFML/Graphics.hpp>
#include <string>
#include <cstdlib>
#include <random>
#include "Matrix.h"
#include "NeuralNetwork.h"
using namespace sf;

float g = 1.4;

int geverty(int v,int t) {
	return v + g * t;
}
void selection(vector<float> score, vector<NeuralNetwork> bird) {
	// 基于适应度选择前5个最好的小鸟
	vector<pair<float, int>> scoreIndex;
	for (int i = 0; i < 100; i++) {
		scoreIndex.push_back({score[i], i});
	}
	// 按得分降序排序
	sort(scoreIndex.begin(), scoreIndex.end(), [](const pair<float, int>& a, const pair<float, int>& b) {
		return a.first > b.first;
	});
	// 保存前25个最好的小鸟
	for (int i = 0; i < 25; i++) {
		bird[scoreIndex[i].second].saveModel(3, i);
	}

}

int main()
{
	//游戏窗口部分
	srand(time(0));//随机数

	sf::Text textFly,textGameOver, textScore;
	sf::Font font;
	font.loadFromFile("arial.ttf");
	textFly.setFont(font);
	textFly.setString("Press W to fly!");
	textFly.setCharacterSize(48);
	textFly.setFillColor(sf::Color::Blue);
	textFly.setStyle(sf::Text::Bold);
	textFly.setPosition(100, 270);

	textGameOver.setFont(font);
	textGameOver.setString("      GameOver!\nPress W to restart!");
	textGameOver.setCharacterSize(48);
	textGameOver.setFillColor(sf::Color::Red);
	textGameOver.setStyle(sf::Text::Bold);
	textGameOver.setPosition(50, 270);


	int scale=2;

	RenderWindow window(VideoMode(256* scale, 384* scale), "Flappy Bird");
	Texture text;
	text.loadFromFile("bird.png");

	Sprite/*指可移动图像*/s(text);
	s.setTextureRect(IntRect(0, 0, 34, 24));
	s.setScale(scale, scale);

	Texture background;
	background.loadFromFile("background.png");

	Sprite b1(background);
	Sprite b2(background);
	b1.setTextureRect(IntRect(0, 0, 256, 384));
	b2.setTextureRect(IntRect(0, 0, 256, 384));
	int backgroudMove = 0;
	b1.setScale(scale, scale);
	b2.setScale(scale, scale);

	Texture pipe;
	pipe.loadFromFile("pipe.png");

	Sprite upPipe(pipe);
	Sprite downPipe(pipe);
	upPipe.setTextureRect(IntRect(0, 0, 60, 384));
	downPipe.setTextureRect(IntRect(0, 0, 60, 384));
	int pipeMove = 0;
	upPipe.setScale(scale, scale);
	downPipe.setScale(scale, scale);

	upPipe.setRotation(180);

	float tickTime = 0, aiTime = 0, delay = 0.03;

	Clock clock;

	int upPipeArea = (20 + rand() % 200)* scale;
	int downPipeArea= upPipeArea+100 * scale;
	bool flyup = true;
	
	bool aiplay = false;
	std::cout << "输入A让ai训练:";
	char c;
	std::cin >> c;
	if (c == 'A') {
		aiplay = true;
	}

	if (!aiplay) {
		textScore.setFont(font);
		int score = 0;
		std::string scoreString = "0";
		textScore.setString(scoreString);
		textScore.setCharacterSize(32);
		textScore.setFillColor(sf::Color::Yellow);
		textScore.setStyle(sf::Text::Bold);
		textScore.setPosition(240, 5);

		bool pass = false;
		bool gameStart = false, gameOver = false;
		int state = 0;
		int birdHeight = (128 - 24) * scale;
		int birdx = (68 + 34) * scale;
		int v = 0;
		int datalX = 256 * scale - pipeMove - birdx,
			datalYup = birdHeight - upPipeArea,
			datalYdowm = downPipeArea - (birdHeight + 24 * scale);
		while (window.isOpen())
		{

			float time = clock.getElapsedTime().asSeconds();
			clock.restart();
			tickTime += time;

			Event gameEvent;
			while (window.pollEvent(gameEvent)) {
				if (gameEvent.type == Event::Closed) {
					window.close();
				}
				if (gameEvent.type == Event::KeyPressed) {
					if (!gameStart) {
						if (gameEvent.key.code == Keyboard::W) {
							gameStart = true;
						}
						else {
							break;
						}
					}
					if (gameOver) {
						if (gameEvent.key.code == Keyboard::W) {
							gameOver = false;	gameStart = false;
							state = 0;	flyup = true;
							upPipeArea = (20 + rand() % 200) * scale;
							downPipeArea = upPipeArea + 100 * scale;
							birdHeight = (128 - 24) * scale;
							birdx = (68 + 34) * scale;
							backgroudMove = 0;	pipeMove = 0;
							v = 0;
						}
						else {
							break;
						}
					}
					if (gameEvent.key.code == Keyboard::W) {
						v -= 10;
						birdHeight += v;
					}
				}
			}
			if (!gameStart) {
				tickTime = 0;
				window.clear(Color::White);
				b1.setPosition(0, 0);
				window.draw(b1);
				s.setTextureRect(IntRect(0, 0, 34, 24));
				s.setPosition(birdx - 34 * scale, birdHeight);
				window.draw(s);
				window.draw(textFly);
				window.display();
				continue;
			}
			if (gameOver) {
				tickTime = 0;
				window.clear(Color::White);
				b1.setPosition(0 - backgroudMove, 0);
				window.draw(b1);
				b2.setPosition(256 * scale - backgroudMove, 0);
				window.draw(b2);
				downPipe.setPosition(256 * scale - pipeMove, downPipeArea);
				window.draw(downPipe);
				upPipe.setPosition(256 * scale - pipeMove + 60 * scale, upPipeArea);
				window.draw(upPipe);
				state = 3;
				score = 0;
				scoreString = std::to_string(score);
				textScore.setString(scoreString);
				s.setTextureRect(IntRect(34 * state, 0, 34, 24));
				s.setPosition(birdx - 34 * scale, birdHeight);
				window.draw(s);
				window.draw(textGameOver);
				window.display();
				continue;
			}
			if (tickTime > delay) {
				tickTime = 0;
				if (flyup) {
					state++;
					if (state == 3) {
						state = 1;
						flyup = false;
						v = geverty(v, 1);
						birdHeight += v;
					}
				}
				else {
					state--;
					if (state == -1) {
						state = 1;
						flyup = true;
						v = geverty(v, 1);
						birdHeight += v;
					}
				}
				backgroudMove += 2;
				pipeMove += 4;
				if (backgroudMove >= 256 * scale)backgroudMove = 0;
				if (pipeMove >= (256 + 60) * scale) {
					upPipeArea = (20 + rand() % 200) * scale;
					downPipeArea = upPipeArea + 100 * scale;
					pipeMove = 0;
					pass = false;
				}
			}

			s.setRotation(v);

			//绘制
			window.clear(Color::White);
			b1.setPosition(0 - backgroudMove, 0);
			window.draw(b1);
			b2.setPosition(256 * scale - backgroudMove, 0);
			window.draw(b2);

			downPipe.setPosition(256 * scale - pipeMove, downPipeArea);
			window.draw(downPipe);
			upPipe.setPosition(256 * scale - pipeMove + 60 * scale, upPipeArea);
			window.draw(upPipe);

			datalX = 256 * scale - pipeMove - birdx;
			datalYup = birdHeight - upPipeArea;
			datalYdowm = downPipeArea - (birdHeight + 24 * scale);

			if ((datalYdowm <= 0 || datalYup <= 0) && (datalX <= 0 && 0 - 34 * scale <= datalX + 60 * scale)) {
				state = 3;
				gameOver = true;
			}
			if (birdHeight <= 0 || birdHeight >= 384 * scale - 24 * scale) {
				state = 3;
				gameOver = true;
			}
			if (!pass && (birdx - 34 * scale >= 256 * scale - pipeMove + 60 * scale)) {
				pass = true;
				score++;
				scoreString = std::to_string(score);
				textScore.setString(scoreString);
			}

			s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
			s.setPosition(birdx - 34 * scale, birdHeight);
			window.draw(s);
			window.draw(textScore);

			window.display();
			if (state == 3) {
				state = 0;
				flyup = true;
			}
		}
	}
	else {
		int birdNum = 100;
		bool trainStart = false;
		vector<NeuralNetwork> bird(birdNum);
		for (int i = 0; i < birdNum; i++) {
			bird[i].initNet(3);
		}
		vector<Matrix> birdState(birdNum);
		vector<Sprite>birds(birdNum);
		vector<bool> pass(birdNum);
		bool AllBirdDead = false;
		vector<bool> birdDead(birdNum);
		vector<int> state(birdNum);
		vector<int> birdHeights(birdNum);
		vector<int> birdxs(birdNum);
		vector<int> v(birdNum);
		vector<int> datalX(birdNum),datalYup(birdNum),datalYdowm(birdNum);
		vector<float> score(birdNum);
		for (int i = 0; i < birdNum; i++) {
			birds[i] = Sprite(text);
			birds[i].setTextureRect(IntRect(0, 0, 34, 24));	
			birds[i].setScale(scale, scale);
			state[i]=0;
			birdDead[i] = false;

			birdxs[i] = (68 + 34) * scale;
			birdHeights[i]= (384/2 - 24) * scale;
			v[i]=0;
			datalX[i] = 256 * scale - pipeMove - birdxs[i];
			datalYup[i]= birdHeights[i] - upPipeArea;
			datalYdowm[i]= downPipeArea - (birdHeights[i] + 24 * scale);
			// 初始状态归一化
			float normalizedHeight = (birdHeights[i] / (384.0 * scale - 24.0 * scale)) * 2.0 - 1.0;
			float normalizedVelocity = v[i] / 20.0 + 10;
			float normalizedDistance = datalX[i] / (256.0 * scale);
			float normalizedUpDistance = datalYup[i] / (100.0 * scale);
			float normalizedDownDistance = datalYdowm[i] / (100.0 * scale);
			birdState[i] = Matrix(5, 0, 0);
			birdState[i].setValue(1, 1, normalizedHeight);
			birdState[i].setValue(2, 1, normalizedVelocity);
			birdState[i].setValue(3, 1, normalizedDistance);
			birdState[i].setValue(4, 1, normalizedUpDistance);
			birdState[i].setValue(5, 1, normalizedDownDistance);
			//birdState[i].show();
		}

		while (window.isOpen())
		{
			float time = clock.getElapsedTime().asSeconds();
			clock.restart();
			tickTime += time;
			aiTime += time;

			Event gameEvent;
			while (window.pollEvent(gameEvent)) {
				if (gameEvent.type == Event::Closed) {
					window.close();
				}
				if (gameEvent.type == Event::Closed) {
					window.close();
				}
				if (gameEvent.type == Event::KeyPressed) {
					if (!trainStart) {
						if (gameEvent.key.code == Keyboard::W) {
							trainStart = true;
						}
						else {
							break;
						}
					}
				}
			}
			if (AllBirdDead) {

				AllBirdDead = false;	
				flyup = true;
				upPipeArea = (20 + rand() % 200) * scale;
				downPipeArea = upPipeArea + 100 * scale;

				backgroudMove = 0;	pipeMove = 0;

				for (int i = 0; i < birdNum; i++) {
					state[i] = 0;
					birdDead[i] = false;
					birdxs[i] = (68 + 34) * scale;
					birdHeights[i] = (384 / 2 - 24) * scale;
					v[i] = 0;
					datalX[i] = 256 * scale - pipeMove - birdxs[i];
					datalYup[i] = birdHeights[i] - upPipeArea;
					datalYdowm[i] = downPipeArea - (birdHeights[i] + 24 * scale);
					// 重新开始时的状态归一化
					float normalizedHeight = (birdHeights[i] / (384.0 * scale - 24.0 * scale)) * 2.0 - 1.0;
					float normalizedVelocity = v[i] / 20.0 + 10;
					float normalizedDistance = datalX[i] / (256.0 * scale);
					float normalizedUpDistance = datalYup[i] / (100.0 * scale);
					float normalizedDownDistance = datalYdowm[i] / (100.0 * scale);
					birdState[i].setValue(0, 0, normalizedHeight);
					birdState[i].setValue(1, 0, normalizedVelocity);
					birdState[i].setValue(2, 0, normalizedDistance);
					birdState[i].setValue(3, 0, normalizedUpDistance);
					birdState[i].setValue(4, 0, normalizedDownDistance);
				}
			}
			if (!trainStart) {
				tickTime = 0;
				window.clear(Color::White);
				b1.setPosition(0, 0);
				window.draw(b1);
				window.draw(textFly);
				window.display();
				continue;
			}

			if (tickTime > delay) {
				tickTime = 0;
				if (flyup) {
					for (int i = 0; i < birdNum; i++) {
						if (birdDead[i]) {
							continue;
						}
						state[i]++;
						if (state[i] == 3) {
							state[i] = 1;
							flyup = false;
							v[i] = geverty(v[i], 1);
							birdHeights[i] += v[i];
							
						}
						//score[i] += 0.01;
					}
				}
				else {
					for (int i = 0; i < birdNum; i++) {
						if (birdDead[i]) {
							continue;
						}
						state[i]--;
						if (state[i] == -1) {
							state[i] = 1;
							flyup = true;
							v[i] = geverty(v[i], 1);
							birdHeights[i] += v[i];
							
						}
						//score[i] += 0.01;
					}
				}
				backgroudMove += 2;
				pipeMove += 4;
				if (backgroudMove >= 256 * scale)backgroudMove = 0;
				if (pipeMove >= (256 + 60) * scale) {
					upPipeArea = (20 + rand() % 200) * scale;
					downPipeArea = upPipeArea + 100 * scale;
					pipeMove = 0;
					for (int i = 0; i < 10; i++) {
						pass[i] = false;
					}
				}
			}
			for (int i = 0; i < birdNum; i++) {
				if (birdDead[i]) {
					continue;
				}
				birds[i].setRotation(v[i]);
			}
			if (aiTime > 0.25) {
				aiTime = 0;
				for (int i = 0; i < birdNum; i++) {
					int choice = bird[i].play(birdState[i]);
					if (choice == 1) {
						v[i] -= 10;
						birdHeights[i] += v[i];
					}
					cout << "bird" << i << " choose " << choice << endl;
				}
			}


			window.clear(Color::White);
			b1.setPosition(0 - backgroudMove, 0);
			window.draw(b1);
			b2.setPosition(256 * scale - backgroudMove, 0);
			window.draw(b2);
			downPipe.setPosition(256 * scale - pipeMove, downPipeArea);
			window.draw(downPipe);
			upPipe.setPosition(256 * scale - pipeMove + 60 * scale, upPipeArea);
			window.draw(upPipe);

			for (int i = 0; i < birdNum; i++) {
				if (birdDead[i]) {
					continue;
				}
				datalX[i] = 256 * scale - pipeMove - birdxs[i];
				datalYup[i] = birdHeights[i] - upPipeArea;
				datalYdowm[i] = downPipeArea - (birdHeights[i] + 24 * scale);

				if ((datalYdowm[i] <= 0 || datalYup[i] <= 0) && (datalX[i] <= 0 && 0 - 34 * scale <= datalX[i] + 60 * scale)) {
					state[i] = 3;
					birdDead[i] = true;
					score[i] -= 10;
				}
				if (birdHeights[i] <= 0 || birdHeights[i] >= 384 * scale - 24 * scale) {
					state[i] = 3;
					birdDead[i] = true;
					score[i] -= 10;
				}
				if (!pass[i] && (birdxs[i] - 34 * scale >= 256 * scale - pipeMove + 60 * scale)) {
					pass[i] = true;
					score[i]++;
				}

				// 优化状态输入的缩放方式
				// 归一化到 -1 到 1 范围
				float normalizedHeight = (birdHeights[i] / (384.0 * scale - 24.0 * scale)) * 2.0 - 1.0;
				float normalizedVelocity = v[i] / 20.0 + 10;
				float normalizedDistance = datalX[i] / (256.0 * scale);
				float normalizedUpDistance = datalYup[i] / (100.0 * scale);
				float normalizedDownDistance = datalYdowm[i] / (100.0 * scale);

				birdState[i].setValue(0, 0, normalizedHeight);
				birdState[i].setValue(1, 0, normalizedVelocity);
				birdState[i].setValue(2, 0, normalizedDistance);
				birdState[i].setValue(3, 0, normalizedUpDistance);
				birdState[i].setValue(4, 0, normalizedDownDistance);

				birds[i].setTextureRect(IntRect(34 * state[i], 0, 34, 24)); // 矩形范围
				birds[i].setPosition(birdxs[i] - 34 * scale, birdHeights[i]);
				window.draw(birds[i]);
			}
			int count = 0;
			for (int i = 0; i < birdNum; i++) {
				if (birdDead[i]) {
					count++;
				}
			}
			if (count == birdNum) {
				AllBirdDead = true;
				selection(score, bird);
				srand(std::time(0));
				for (int i = 0; i < birdNum; i++) {
					bird[i].crossoverAndmutation(3, rand() % 25, (rand() % 25 + 3) % 25);
				}
			}
			window.display();

		}
	}

	return 0;
}



