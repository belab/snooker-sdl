#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;
const int BALL_RADIUS = 10;
const double FRICTION = 0.9995;
const int TABLE_MARGIN = 60;
const int POCKET_RADIUS = 20;
const double TRIANGLE_START_X = SCREEN_WIDTH * 0.75;
const double TRIANGLE_START_Y = SCREEN_HEIGHT / 2.0;
const double SPACING = BALL_RADIUS * 2 + 2;

struct Vec2d
{
	double x{}, y{};
	constexpr double squaredLength() const {return dot(*this);}
	constexpr double dot(const Vec2d& rhs) const {return x * rhs.x + y * rhs.y;}
	constexpr Vec2d norm() const {return *this / std::sqrt(squaredLength());}
	constexpr Vec2d operator+(const Vec2d& rhs) const {return Vec2d{x + rhs.x, y + rhs.y};};
	constexpr Vec2d operator-(const Vec2d& rhs) const {return Vec2d{x - rhs.x, y - rhs.y};};
	constexpr Vec2d operator*(double multiplier) const {return Vec2d{x * multiplier, y * multiplier};};
	constexpr Vec2d operator/(double divisor) const {return Vec2d{x / divisor, y / divisor};};
	constexpr Vec2d& operator+=(const Vec2d& rhs) {x += rhs.x; y += rhs.y; return *this;};
	constexpr Vec2d& operator-=(const Vec2d& rhs) {x -= rhs.x; y -= rhs.y; return *this;};
	constexpr Vec2d& operator*=(double multiplier) {x *= multiplier; y *= multiplier; return *this;};
};

struct Color
{
    Uint8 r{};
    Uint8 g{};
    Uint8 b{};
};

struct Ball {
	Vec2d pos;
	Vec2d v;
	Color color;
	int points; // Scoring value of the ball
	bool potted = false;
	std::string name;
};

Ball createBall(const std::string& ballName)
{
	const std::unordered_map<std::string, Ball> balls =
	{
		{"Yellow Ball", Ball{{SCREEN_WIDTH / 2.0 - 3 * SPACING, SCREEN_HEIGHT / 2.0}, 0, 0, {255, 255, 0}, 2, false, "Yellow Ball"}},
		{"Green Ball", Ball{{SCREEN_WIDTH / 2.0 - SPACING, TABLE_MARGIN + SPACING * 2}, 0, 0, {0, 255, 0}, 3, false, "Green Ball"}},
		{"Brown Ball", Ball{{SCREEN_WIDTH / 2.0 - SPACING, SCREEN_HEIGHT - TABLE_MARGIN - SPACING * 2}, 0, 0, {139, 69, 19}, 4, false, "Brown Ball"}},
		{"Blue Ball", Ball{{SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0}, 0, 0, {0, 0, 255}, 5, false, "Blue Ball"}},
		{"Pink Ball", Ball{{TRIANGLE_START_X - SPACING * 3, SCREEN_HEIGHT / 2.0}, 0, 0, {255, 105, 180}, 6, false, "Pink Ball"}},
		{"Black Ball", Ball{{TRIANGLE_START_X + SPACING * 6, SCREEN_HEIGHT / 2.0}, 0, 0, {0, 0, 0}, 7, false, "Black Ball"}},
		{"Cue Ball", Ball{{SCREEN_WIDTH / 4.0, SCREEN_HEIGHT / 2.0}, 0, 0, {255, 255, 255}, 0, false, "Cue Ball"}}
	};

	return balls.at(ballName);
}
struct Pocket {
	Vec2d pos;
};

std::vector<Pocket> createPockets() {
	return {
		{TABLE_MARGIN, TABLE_MARGIN},                                  // Top-left
		{SCREEN_WIDTH / 2.0, TABLE_MARGIN},                          // Top-center
		{SCREEN_WIDTH - TABLE_MARGIN, TABLE_MARGIN},                  // Top-right
		{TABLE_MARGIN, SCREEN_HEIGHT - TABLE_MARGIN},                 // Bottom-left
		{SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - TABLE_MARGIN},          // Bottom-center
		{SCREEN_WIDTH - TABLE_MARGIN, SCREEN_HEIGHT - TABLE_MARGIN},  // Bottom-right
	};
}

void drawBall(SDL_Renderer* renderer, const Ball& ball) {
	if (ball.potted) return; // Do not draw potted balls
	for (int w = 0; w < BALL_RADIUS * 2; w++) {
		for (int h = 0; h < BALL_RADIUS * 2; h++) {
			int dx = BALL_RADIUS - w;
			int dy = BALL_RADIUS - h;
			if (dx * dx + dy * dy <= BALL_RADIUS * BALL_RADIUS) {
				SDL_SetRenderDrawColor(renderer, ball.color.r, ball.color.g, ball.color.b, 255);
				SDL_RenderDrawPoint(renderer, int(ball.pos.x + dx), int(ball.pos.y + dy));
			}
		}
	}
}

void drawPockets(SDL_Renderer* renderer, const std::vector<Pocket>& pockets) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	for (const auto& pocket : pockets) {
		for (int w = 0; w < POCKET_RADIUS * 2; w++) {
			for (int h = 0; h < POCKET_RADIUS * 2; h++) {
				int dx = POCKET_RADIUS - w;
				int dy = POCKET_RADIUS - h;
				if (dx * dx + dy * dy <= POCKET_RADIUS * POCKET_RADIUS) {
					SDL_RenderDrawPoint(renderer, int(pocket.pos.x) + dx, int(pocket.pos.y) + dy);
				}
			}
		}
	}
}

bool isBallInPocket(const Ball& ball, const Pocket& pocket) {
	return (ball.pos - pocket.pos).squaredLength() < (POCKET_RADIUS*POCKET_RADIUS);
}

void handlePotting(Ball& ball, const std::vector<Pocket>& pockets, int& score) {
	if (ball.potted) return;
	for (const auto& pocket : pockets) {
		if (isBallInPocket(ball, pocket)) {
			ball.potted = true;

			if (ball.name == "Cue Ball") {
				std::cout << "Cue Ball potted! Foul!\n";
			} else if (ball.points > 1) {
				std::cout << ball.name << " potted!\n";
			} else {
				// Score red balls (do not respawn)
				score += ball.points;
				std::cout << ball.name << " potted! Score: " << score << "\n";
			}
			return;
		}
	}
}


void handlePottedBalls(std::vector<Ball>& balls) {
	for(auto& ball : balls) {
		if(!ball.potted) continue;
		if (ball.name == "Cue Ball") {
			// Reset the cue ball to its original position
			ball = createBall(ball.name);
			std::cout << "Respawn " << ball.name << "\n";
		} else if (ball.points > 1) {
			// Reset colored balls to their original positions
			ball = createBall(ball.name);
			std::cout << "Respawn " << ball.name << "\n";
		} else {
			// Score red balls (do not respawn)
		}
	}
}

bool isMoving(const Ball& ball)
{
	if(ball.v.squaredLength() < 0.0001)	{
		return false;
	}
	return true;
}

bool updateBall(Ball& ball) {
	if(ball.potted || !isMoving(ball)) {
		return false;
	}
	// Apply velocity
	ball.pos += ball.v;

	// Apply friction
	ball.v *= FRICTION;

	// Bounce off table edges
	if (ball.pos.x - BALL_RADIUS < TABLE_MARGIN || ball.pos.x + BALL_RADIUS > SCREEN_WIDTH - TABLE_MARGIN) {
		ball.v.x = -ball.v.x;
		ball.pos.x = std::clamp(ball.pos.x, (double)(TABLE_MARGIN + BALL_RADIUS), (double)(SCREEN_WIDTH - TABLE_MARGIN - BALL_RADIUS));
	}
	if (ball.pos.y - BALL_RADIUS < TABLE_MARGIN || ball.pos.y + BALL_RADIUS > SCREEN_HEIGHT - TABLE_MARGIN) {
		ball.v.y = -ball.v.y;
		ball.pos.y = std::clamp(ball.pos.y, (double)(TABLE_MARGIN + BALL_RADIUS), (double)(SCREEN_HEIGHT - TABLE_MARGIN - BALL_RADIUS));
	}
	return true;
}

void handleCollision(Ball& a, Ball& b) {
	if(a.potted || b.potted) return;
	if(!isMoving(a) && !isMoving(b)) return;
	Vec2d ab = b.pos - a.pos;

	double distance = std::sqrt(ab.squaredLength());
	if (distance < BALL_RADIUS * 2) {
		// Resolve overlap by backward moving both along abNorm by half the overlap.
		double overlap = BALL_RADIUS * 2 - distance;
		Vec2d abNorm = ab.norm();
		a.pos -= abNorm * overlap * 0.5;
		b.pos += abNorm * overlap * 0.5;

		// Elastic Collision: When two objects collide elastically, they exchange momentum along the line of impact.
		// The component of the relative velocity (a.v - b.v) along abNorm
		Vec2d v = abNorm * (a.v - b.v).dot(abNorm); // scalar projection of the relative velocity onto abNorm.
		a.v -= v; // reduces its velocity in the collision direction
		b.v += v; // increases its velocity in the collision direction
	}
}

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("Snooker Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}

	// Pockets
	std::vector<Pocket> pockets = createPockets();

	// Initialize balls with snooker-specific arrangement
	std::vector<Ball> balls;
	int score = 0;

	// Cue ball
	balls.push_back(createBall("Cue Ball"));

	// Red balls (1 point each)
	for (int row = 0; row < 5; ++row) {
		for (int col = 0; col <= row; ++col) {
			balls.push_back({TRIANGLE_START_X + row * SPACING, TRIANGLE_START_Y - row * SPACING / 2 + col * SPACING, 0, 0, {255, 0, 0}, 1, false, "Red Ball"});
		}
	}

	// Colored balls
	balls.push_back(createBall("Yellow Ball"));
	balls.push_back(createBall("Green Ball"));
	balls.push_back(createBall("Brown Ball"));
	balls.push_back(createBall("Blue Ball"));
	balls.push_back(createBall("Pink Ball"));
	balls.push_back(createBall("Black Ball"));


	bool running = true;
	bool aiming = false;
	Ball* cueBall = &balls[0];
	double aimAngle = 0.0f;
	bool doRender = true;

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				if (!aiming) {
					aiming = true;
				}
			} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
				if (aiming) {
					Vec2d offset = Vec2d{double(event.button.x), double(event.button.y)} - cueBall->pos;
					cueBall->v = offset * 0.005;
					aiming = false;
				}
			}
		}

		// Update ball positions
		size_t movingBalls = balls.size();
		for (auto& ball : balls) {
			if(!updateBall(ball))
				movingBalls--;
		}

		// Check potting
		for (auto& ball : balls) {
			if(!ball.potted)
				handlePotting(ball, pockets, score);
		}

		if(movingBalls == 0) {
			// std::cout << "balls stopped\n";
			handlePottedBalls(balls);
		}


		// Handle collisions
		for (size_t i = 0; i < balls.size(); i++) {
			for (size_t j = i + 1; j < balls.size(); j++) {
				handleCollision(balls[i], balls[j]);
			}
		}

		// Render the game
		SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
		SDL_RenderClear(renderer);

		// Draw the table border
		SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
		SDL_Rect table = {TABLE_MARGIN, TABLE_MARGIN, SCREEN_WIDTH - 2 * TABLE_MARGIN, SCREEN_HEIGHT - 2 * TABLE_MARGIN};
		SDL_RenderDrawRect(renderer, &table);
		drawPockets(renderer, pockets);

		// Draw balls
		for (const auto& ball : balls) {
			drawBall(renderer, ball);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
