#include <SFML/Graphics.hpp>
using namespace sf;

int main()
{
	srand(time(0));//随机数

	RenderWindow window(VideoMode(256, 384), "Flappy Bird");
	Texture text;
	text.loadFromFile("bird.png");

	Sprite/*指可移动图像*/s(text);
	s.setTextureRect(IntRect(0, 0, 34, 24));

	Texture background;
	background.loadFromFile("background.png");

	Sprite b1(background);
	Sprite b2(background);
	b1.setTextureRect(IntRect(0, 0, 256, 384));
	b2.setTextureRect(IntRect(0, 0, 256, 384));
	int backgroudMove = 0;

	Texture pipe;
	pipe.loadFromFile("pipe.png");

	Sprite upPipe(pipe);
	Sprite downPipe(pipe);
	upPipe.setTextureRect(IntRect(0, 0, 60, 384));
	downPipe.setTextureRect(IntRect(0, 0, 60, 384));
	int pipeMove = 0;

	upPipe.setRotation(180);

	float tickTime = 0, delay = 0.05;

	int state = 0;
	float birdRotate = 0;
	bool flyup = true, rotateup = true;

	Clock clock;

	int upPipeArea= 20 + rand() % 200;
	int downPipeArea= upPipeArea+100;

	int birdHeight = 128 - 24;
	int birdx = 68 + 34;

	while (window.isOpen())
	{

		float time = clock.getElapsedTime().asSeconds();
		clock.restart();
		tickTime += time;


		Event gameEvent;
		while (window.pollEvent(gameEvent))
		{
			if (gameEvent.type == Event::Closed) {
				window.close();
			}
			//按键
			if (gameEvent.type == Event::KeyPressed) {
				if (gameEvent.key.code == Keyboard::W)birdHeight-=7;
			}
		}
		//下落间隔
		if (tickTime > delay) {
			tickTime = 0;
			if (flyup) {
				state++;
				if (state == 3) {
					state = 1;
					flyup = false;
				}
			}
			else {
				state--;
				if (state == -1) {
					state = 1;
					flyup = true;
				}
				birdHeight++;
			}
			/*
			if (rotateup) {
				birdRotate += 5;
				s.rotate(5);
				if (birdRotate == 30) {
					rotateup = false;
				}
			}
			else {
				birdRotate -= 5;
				s.rotate(-5);
				if (birdRotate == -30) {
					rotateup = true;
				}
			}
			*/
			backgroudMove++ ;
			pipeMove+= 2;
			if (backgroudMove == 256)backgroudMove = 0;
			if (pipeMove == 256 + 60) {
				upPipeArea = 20 + rand() % 200;
				downPipeArea = upPipeArea + 100;
				pipeMove = 0;
			}
		}

		//绘制
		window.clear(Color::White);
		b1.setPosition(0 - backgroudMove, 0);
		window.draw(b1);
		b2.setPosition(256 - backgroudMove, 0);
		window.draw(b2);

		downPipe.setPosition(256 - pipeMove, downPipeArea);
		window.draw(downPipe);
		upPipe.setPosition(256 - pipeMove + 60, upPipeArea);
		window.draw(upPipe);

		if ((birdHeight + 24 >= downPipeArea || birdHeight <= upPipeArea) && (birdx >= 256 - pipeMove && birdx <= 256 - pipeMove + 60)) {
			state = 3;
		}


		s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
		s.setPosition(birdx-34, birdHeight);
		window.draw(s);

		window.display();
		if (state == 3) {
			state = 0;
			flyup = true;
		}
	}

	return 0;
}