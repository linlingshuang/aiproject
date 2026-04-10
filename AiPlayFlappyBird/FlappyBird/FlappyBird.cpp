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

	Sprite/*指可移动图像*/b1(background);
	Sprite/*指可移动图像*/b2(background);
	b1.setTextureRect(IntRect(0, 0, 256, 384));
	b2.setTextureRect(IntRect(0, 0, 256, 384));
	int backgroudmove = 0;

	float tickTime = 0, delay = 0.1;

	int state = 0;
	float birdRotate = 0;
	bool flyup = true, rotateup = true;

	Clock clock;

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
			backgroudmove+=2;
			if (backgroudmove == 256)
				backgroudmove = 0;
		}

		//绘制
		window.clear(Color::White);
		b1.setPosition(0 - backgroudmove, 0);
		window.draw(b1);
		b2.setPosition(256 - backgroudmove, 0);
		window.draw(b2);

		s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
		s.setPosition(128 - 34, 128 - 24);
		window.draw(s);

		window.display();

	}

	return 0;
}