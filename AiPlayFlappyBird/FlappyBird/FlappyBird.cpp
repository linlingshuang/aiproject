#include <SFML/Graphics.hpp>
using namespace sf;

float g = 1.2;

int geverty(int v,int t) {
	return v + g * t;
}

int main()
{
	srand(time(0));//随机数

	// 文本对象
	sf::Text textFly,textGameOver;
	sf::Font font;
	font.loadFromFile("arial.ttf");
	textFly.setFont(font);

	textFly.setString("Press W to fly!");
	textFly.setCharacterSize(48);
	textFly.setFillColor(sf::Color::Blue);
	textFly.setStyle(sf::Text::Bold);
	textFly.setPosition(100, 270);

	textGameOver.setFont(font);

	textGameOver.setString("GameOver!Press W to restart!");
	textGameOver.setCharacterSize(48);
	textGameOver.setFillColor(sf::Color::Red);
	textGameOver.setStyle(sf::Text::Bold);
	textGameOver.setPosition(50, 270);

	int scale=2;
	bool gameStart = false;
	bool gameOver = false;

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

	float tickTime = 0, delay = 0.05;

	int state = 0;
	float birdRotate = 0;
	bool flyup = true;

	Clock clock;

	int upPipeArea = (20 + rand() % 200)* scale;
	int downPipeArea= upPipeArea+100 * scale;

	int birdHeight = (128 - 24)* scale;
	int birdx = (68 + 34)* scale;
	int v = 0;

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
						gameOver = false;
						gameStart = false;
						state = 0;
						birdRotate = 0;
						flyup = true;
						upPipeArea = (20 + rand() % 200) * scale;
						downPipeArea = upPipeArea + 100 * scale;
						birdHeight = (128 - 24) * scale;
						birdx = (68 + 34) * scale;
						backgroudMove = 0;
						pipeMove = 0;
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
			b1.setPosition(0 - backgroudMove, 0);
			window.draw(b1);
			b2.setPosition(256 * scale - backgroudMove, 0);
			window.draw(b2);
			downPipe.setPosition(256 * scale - pipeMove, downPipeArea);
			window.draw(downPipe);
			upPipe.setPosition(256 * scale - pipeMove + 60 * scale, upPipeArea);
			window.draw(upPipe);
			s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
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
			s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
			s.setPosition(birdx - 34 * scale, birdHeight);
			window.draw(s);
			window.draw(textGameOver);
			window.display();
			continue;
		}
		//下落间隔
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
					v = geverty(v,1);
					birdHeight += v;
				}
			}
			backgroudMove+=2 ;
			pipeMove+= 4;
			if (backgroudMove == 256* scale)backgroudMove = 0;
			if (pipeMove == (256 + 60)* scale) {
				upPipeArea = (20 + rand() % 200)* scale;
				downPipeArea = upPipeArea + 100 * scale;
				pipeMove = 0;
			}
		}

		s.setRotation(v);

		//绘制
		window.clear(Color::White);
		b1.setPosition(0 - backgroudMove, 0);
		window.draw(b1);
		b2.setPosition(256* scale - backgroudMove, 0);
		window.draw(b2);

		downPipe.setPosition(256* scale - pipeMove, downPipeArea);
		window.draw(downPipe);
		upPipe.setPosition(256* scale - pipeMove + 60* scale, upPipeArea);
		window.draw(upPipe);

		if ((birdHeight + 24* scale >= downPipeArea || birdHeight <= upPipeArea) && (birdx >= 256* scale - pipeMove && birdx <= 256* scale - pipeMove + 60* scale)) {
			state = 3;
			gameOver = true;
		}
		if (birdHeight<=0|| birdHeight>= 384 * scale-24*scale) {
			state = 3;
			gameOver = true;
		}

		s.setTextureRect(IntRect(34 * state, 0, 34, 24)); // 矩形范围
		s.setPosition(birdx-34* scale, birdHeight);
		window.draw(s);

		window.display();
		if (state == 3) {
			state = 0;
			flyup = true;
		}
	}

	return 0;
}