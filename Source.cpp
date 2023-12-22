# include <SFML/Graphics.hpp>
# include <vector>

#define MIN(a,b) a < b ? a : b
#define SCREEN_W 1024
#define SCREEN_H 768
#define PADDING 64
#define MAP_W 36
#define MAP_H 30
#define COLORS 18
#define FONT "DejaVuSans.ttf"

#define sprintf_s(buffer, buffer_size, stringbuffer, ...) (sprintf(buffer, stringbuffer, __VA_ARGS__))

const sf::Color colors[] = {
	sf::Color(0xF44336ff), //Red
	sf::Color(0xE91E63ff), //Pink
	sf::Color(0x9C27B0ff), //Purple
	sf::Color(0x673AB7ff), //Deep Purple
	sf::Color(0x3F51B5ff), //Indigo
	sf::Color(0x2196F3ff), //Blue
	sf::Color(0x03A9F4ff), //Light Blue
	sf::Color(0x00BCD4ff), //Cyan
	sf::Color(0x009688ff), //Teal
	sf::Color(0x4CAF50ff), //Green
	sf::Color(0x8BC34Aff), //Light Green
	sf::Color(0xCDDC39ff), //Lime
	sf::Color(0xFFEB3Bff), //Yellow
	sf::Color(0xFFC107ff), //Amber
	sf::Color(0xFF9800ff), //Orange
	sf::Color(0xFF5722ff), //Deep Orange
	sf::Color(0x795548ff), //Brown
	sf::Color(0x607D8Bff) //Blue Grey
};

sf::RenderWindow* window;
sf::Texture* gridTex, * fieldTex, * rotateIconTex, * skipTurnTex, * paintIconTex;
sf::RectangleShape* gridRect, * fieldRect, * rect, * rotateIconRect, * skipTurnRect, * paintIconRect;
sf::Font* fnt;
sf::Text* text;
sf::Color p1color = sf::Color::Red;
sf::Color p2color = sf::Color::Green;

int cellSize = 0;
int** map;
int skip1 = 0, skip2 = 0;
char buf[128];
int turn = -1;
int rw, rh;
int p1s = 0, p2s = 0;

std::vector<sf::IntRect> p1Rects;
std::vector<sf::IntRect> p2Rects;

bool gameOver = false;
int cx = -1, cy = -1;
int placeX = -1, placeY = -1;
int lastX = -1, lastY = -1;
int p1cInd = -1, p2cInd = -1;

void prerenderGrid() {
	sf::RenderTexture* rTex = new sf::RenderTexture();
	rTex->create(MAP_W * cellSize, MAP_H * cellSize);

	sf::RectangleShape* gridShape = new sf::RectangleShape();

	gridShape->setSize(sf::Vector2f(cellSize - 2, cellSize - 2));
	gridShape->setOutlineColor(sf::Color(66, 66, 66, 255));
	gridShape->setOutlineThickness(1.0f);
	gridShape->setFillColor(sf::Color::Transparent);

	rTex->clear(sf::Color::Transparent);

	for (int i = 0; i < MAP_W; i++) {
		for (int j = 0; j < MAP_H; j++) {
			gridShape->setPosition(sf::Vector2f(cellSize * i + 1, cellSize * j + 1));
			rTex->draw(*gridShape);
		}
	}

	rTex->display();

	gridTex = new sf::Texture(rTex->getTexture());

	delete gridShape;
	delete rTex;
}

void updateFieldTex() {
	sf::RenderTexture* rTex = new sf::RenderTexture();
	rTex->create(MAP_W * cellSize, MAP_H * cellSize);

	float gridw = MAP_W * cellSize;
	float gridh = MAP_H * cellSize;

	rTex->clear(sf::Color::Transparent);
	rTex->draw(*gridRect);

	//P1
	sf::Color col = p1color;
	rect->setOutlineColor(col);
	col.a = 164;
	rect->setFillColor(col);

	text->setOutlineThickness(0.5f);
	text->setCharacterSize(16);
	text->setOutlineColor(sf::Color::White);
	text->setFillColor(sf::Color::White);

	for (auto it = p1Rects.begin(); it != p1Rects.end(); ++it) {
		sf::IntRect r = *it;
		rect->setSize(sf::Vector2f(r.width * cellSize - 4, r.height * cellSize - 4));
		rect->setPosition(sf::Vector2f(cellSize * r.left + 2, cellSize * r.top + 2));
		rTex->draw(*rect);

		//Text
		sprintf_s(buf, 128, "%d", r.width * r.height);
		text->setString(sf::String(buf));
		sf::FloatRect bounds = text->getLocalBounds();

		float tx = (cellSize * r.left) + ((r.width * cellSize) - bounds.width) / 2;
		float ty = (cellSize * r.top) + ((r.height * cellSize) - bounds.height) / 2 - 4;

		text->setPosition(tx, ty);
		rTex->draw(*text);
	}

	//P2
	col = p2color;
	rect->setOutlineColor(col);
	col.a = 164;
	rect->setFillColor(col);

	for (auto it = p2Rects.begin(); it != p2Rects.end(); ++it) {
		sf::IntRect r = *it;
		rect->setSize(sf::Vector2f(r.width * cellSize - 4, r.height * cellSize - 4));
		rect->setPosition(sf::Vector2f(cellSize * r.left + 2, cellSize * r.top + 2));
		rTex->draw(*rect);

		//Text
		sprintf_s(buf, 128, "%d", r.width * r.height);
		text->setString(sf::String(buf));
		sf::FloatRect bounds = text->getLocalBounds();

		float tx = (cellSize * r.left) + ((r.width * cellSize) - bounds.width) / 2;
		float ty = (cellSize * r.top) + ((r.height * cellSize) - bounds.height) / 2 - 4;

		text->setPosition(tx, ty);
		rTex->draw(*text);
	}


	rTex->display();
	fieldTex = new sf::Texture(rTex->getTexture());

	delete rTex;
}

void changeColors() {
	p1cInd = rand() % COLORS;
	p1color = colors[p1cInd];
	do {
		p2cInd = rand() % COLORS;
		p2color = colors[p2cInd];
	} while (p1cInd == p2cInd);
}

void changeActivePlayerColor() {
	int player = turn % 2;
	if (player == 0) {
		do {
			p1cInd = rand() % COLORS;
			p1color = colors[p1cInd];
		} while (p1cInd == p2cInd);
	}
	else {
		do {
			p2cInd = rand() % COLORS;
			p2color = colors[p2cInd];
		} while (p1cInd == p2cInd);
	}

	updateFieldTex();
	fieldRect->setTexture(fieldTex, true);
}

void nextTurn() {
	turn++;
	rw = 1 + rand() % 6;
	rh = 1 + rand() % 6;
}

bool canPlace(int x, int y) {
	if (x < 0 || y < 0 || x > MAP_W - rw || y > MAP_H - rh) {
		return false;
	}

	int player = turn % 2;
	if (player == 0 && p1Rects.size() == 0 && x == 0 && y == 0) {
		return true;
	}
	if (player == 1 && p2Rects.size() == 0 && x == MAP_W - rw && y == MAP_H - rh) {
		return true;
	}

	for (int i = x; i < x + rw; i++) {
		for (int j = y; j < y + rh; j++) {
			if (map[i][j] == 0 || map[i][j] == 1) {
				return false;
			}
		}
	}

	for (int i = x; i < x + rw; i++) {
		if (y - 1 >= 0 && map[i][y - 1] == player) {
			return true;
		}
		if (y + rh < MAP_H && map[i][y + rh] == player) {
			return true;
		}
	}

	for (int i = y; i < y + rh; i++) {
		if (x - 1 >= 0 && map[x - 1][i] == player) {
			return true;
		}
		if (x + rw < MAP_W && map[x + rw][i] == player) {
			return true;
		}
	}

	return false;
}

void place(int x, int y) {
	if (x < 0 || y < 0 || x > MAP_W - rw || y > MAP_H - rh) {
		return;
	}

	int player = turn % 2;
	for (int i = x; i < x + rw; i++) {
		for (int j = y; j < y + rh; j++) {
			map[i][j] = player;
		}
	}

	if (player == 0) {
		p1Rects.push_back(sf::IntRect(x, y, rw, rh));
		p1s += rw * rh;
		skip1 = 0;
	}
	else {
		p2Rects.push_back(sf::IntRect(x, y, rw, rh));
		p2s += rw * rh;
		skip2 = 0;
	}

	updateFieldTex();
	fieldRect->setTexture(fieldTex, true);
	nextTurn();
}

void restart() {
	for (int i = 0; i < MAP_W; i++) {
		for (int j = 0; j < MAP_H; j++) {
			map[i][j] = -1;
		}
	}

	p1Rects.clear();
	p2Rects.clear();

	p1s = 0;
	p2s = 0;
	turn = 0;
	skip1 = 0;
	skip2 = 0;

	changeColors();
	updateFieldTex();

	fieldRect->setTexture(fieldTex, true);
	gameOver = false;
}

int main() {
	srand((unsigned)time(0));

	cellSize = MIN((SCREEN_W - PADDING * 2) / MAP_W, (SCREEN_H - PADDING * 2) / MAP_H);
	map = new int* [MAP_W];

	for (int i = 0; i < MAP_W; i++)
		map[i] = new int[MAP_H];

	for (int i = 0; i < MAP_W; i++) {
		for (int j = 0; j < MAP_H; j++) {
			map[i][j] = -1;
		}
	}

	prerenderGrid();

	changeColors();

	fnt = new sf::Font();
	fnt->loadFromFile(FONT);
	text = new sf::Text();
	text->setFont(*fnt);

	rotateIconTex = new sf::Texture();
	rotateIconTex->loadFromFile("rotate.png");

	skipTurnTex = new sf::Texture();
	skipTurnTex->loadFromFile("skip.png");

	paintIconTex = new sf::Texture();
	paintIconTex->loadFromFile("fill.png");

	float gridx = (SCREEN_W - MAP_W * cellSize) / 2;
	float gridy = (SCREEN_H - MAP_H * cellSize) / 2;
	float gridw = MAP_W * cellSize;
	float gridh = MAP_H * cellSize;

	window = new sf::RenderWindow(sf::VideoMode(SCREEN_W, SCREEN_H), "Cubes");
	window->setFramerateLimit(60);

	rect = new sf::RectangleShape();
	rect->setOutlineThickness(2.0f);

	gridRect = new sf::RectangleShape();
	gridRect->setSize(sf::Vector2f(gridw, gridh));
	gridRect->setTexture(gridTex, true);
	gridRect->setPosition(sf::Vector2f(0, 0));

	updateFieldTex();

	fieldRect = new sf::RectangleShape();
	fieldRect->setSize(sf::Vector2f(gridw, gridh));
	fieldRect->setTexture(fieldTex, true);
	fieldRect->setPosition(sf::Vector2f(gridx, gridy));

	rotateIconRect = new sf::RectangleShape();
	rotateIconRect->setTexture(rotateIconTex);
	rotateIconRect->setSize(sf::Vector2f(32, 32));
	rotateIconRect->setPosition(10, 64);

	skipTurnRect = new sf::RectangleShape();
	skipTurnRect->setTexture(skipTurnTex);
	skipTurnRect->setSize(sf::Vector2f(32, 32));
	skipTurnRect->setPosition(10, 106);

	paintIconRect = new sf::RectangleShape();
	paintIconRect->setTexture(paintIconTex);
	paintIconRect->setSize(sf::Vector2f(32, 32));
	paintIconRect->setPosition(10, 148);

	nextTurn();
	while (window->isOpen()) {
		sf::Event evt;
		while (window->pollEvent(evt)) {
			if (evt.type == sf::Event::Closed) {
				window->close();
			}

			else if (evt.type == sf::Event::MouseMoved) {

				if (fieldRect->getGlobalBounds().contains(evt.mouseMove.x, evt.mouseMove.y)) {
					int x = (evt.mouseMove.x - gridx) / cellSize;
					int y = (evt.mouseMove.y - gridy) / cellSize;

					cx = x;
					cy = y;

					if (x != lastX || y != lastY) {
						lastX = x;
						lastY = y;

						if (canPlace(x, y)) {
							placeX = x;
							placeY = y;
						}
						else {
							placeX = -1;
							placeY = -1;
						}
					}
				}
				else {
					lastX = -1;
					lastY = -1;
					placeX = -1;
					placeY = -1;
					cx = -1;
					cy = -1;
				}
			}

			else if (evt.type == sf::Event::MouseButtonReleased) {
				if (evt.mouseButton.button == sf::Mouse::Button::Left) {
					if (!gameOver) {
						if (fieldRect->getGlobalBounds().contains(evt.mouseButton.x, evt.mouseButton.y)) {
							if (placeX != -1 && placeY != -1) {
								place(placeX, placeY);
								placeX = -1;
								placeY = -1;
							}
						}
						// Новые условия (для пунктов 2,3,4)
						else if (rotateIconRect->getGlobalBounds().contains(evt.mouseButton.x, evt.mouseButton.y)) {
							int tmp = rh;
							rh = rw;
							rw = tmp;
						}

						else if (paintIconRect->getGlobalBounds().contains(evt.mouseButton.x, evt.mouseButton.y)) {
							changeActivePlayerColor();
						}

						else if (skipTurnRect->getGlobalBounds().contains(evt.mouseButton.x, evt.mouseButton.y)) {
							int player = turn % 2;
							if (player == 0)
								skip1++;
							if (player == 1)
								skip2++;
							if (skip1 >= 3 && skip2 >= 3) {
								gameOver = true;
							}

							else {
								nextTurn();
							}
						}
					}
					else {
						restart();
					}
				}
			}

			else if (evt.type == sf::Event::KeyReleased) {
				if (evt.key.code == sf::Keyboard::Space) {
					int tmp = rh;
					rh = rw;
					rw = tmp;

					if (cx != -1 && cy != -1) {
						if (canPlace(cx, cy)) {
							placeX = cx;
							placeY = cy;
						}
						else {
							placeX = -1;
							placeY = -1;
						}
					}
				}
				if (evt.key.code == sf::Keyboard::R) {
					restart();
				}

				if (evt.key.code == sf::Keyboard::S) {
					if (!gameOver) {
						int player = turn % 2;
						if (player == 0)
							skip1++;
						if (player == 1)
							skip2++;
						if (skip1 >= 3 && skip2 >= 3) {
							gameOver = true;
						}

						else {
							nextTurn();
						}
					}
					else {
						restart();
					}
				}
				if (evt.key.code == sf::Keyboard::C) {
					changeActivePlayerColor();
				}
			}
		}

		window->clear();
		window->draw(*fieldRect);

		if (cx >= 0 && cx <= MAP_W - rw && cy >= 0 && cy <= MAP_H - rh) {
			sf::Color col = sf::Color::White;
			rect->setOutlineColor(col);
			col.a = 128;
			rect->setFillColor(col);
			rect->setSize(sf::Vector2f(rw * cellSize - 4, rh * cellSize - 4));
			rect->setPosition(sf::Vector2f(gridx + cellSize * cx + 2, gridy + cellSize * cy + 2));
			window->draw(*rect);
		}

		//UI
		window->draw(*rotateIconRect);
		window->draw(*skipTurnRect);
		window->draw(*paintIconRect);

		text->setFillColor(sf::Color::White);
		text->setCharacterSize(26);
		text->setOutlineThickness(0);

		text->setFillColor(turn % 2 == 0 ? p1color : p2color);
		sprintf_s(buf, 128, "Turn: %d, Player: %d", turn + 1, turn % 2 + 1);
		text->setString(sf::String(buf));
		text->setPosition(sf::Vector2f(10, 10));
		window->draw(*text);

		sprintf_s(buf, 128, "Rect: %dx%d", rw, rh);
		text->setString(sf::String(buf));
		text->setPosition(sf::Vector2f(10, 32));
		window->draw(*text);

		text->setFillColor(p1color);
		sprintf_s(buf, 128, "Player 1 square: %d (Skips: %d)", p1s, skip1);
		text->setString(sf::String(buf));
		text->setPosition(sf::Vector2f(10, SCREEN_H - 60));
		window->draw(*text);

		text->setFillColor(p2color);
		sprintf_s(buf, 128, "Player 2 square: %d (Skips: %d)", p2s, skip2);
		text->setString(sf::String(buf));
		text->setPosition(sf::Vector2f(10, SCREEN_H - 38));
		window->draw(*text);

		window->display();
	}

	for (int i = 0; i < MAP_W; i++)
		delete[] map[i];
	delete[] map;

	delete text;
	delete fnt;
	delete rotateIconRect;
	delete rotateIconTex;
	delete rect;
	delete gridRect;
	delete fieldRect;
	delete gridTex;
	delete fieldTex;
	delete window;

	return 0;
}
