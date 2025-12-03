#include "Entity.h"
#include <MMath.h>

Entity::Entity(): // this is an initializer list
	surface(nullptr), 
	texture(nullptr), 
	angleDeg(0.0f), 
	mass(1.0f),
	r(1.0f),
	speed(70.0f)
{
}

Entity::~Entity() {
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	surface = nullptr;
	texture = nullptr;
}


void Entity::SetImage(const char* filename, SDL_Renderer* renderer) {
	surface = IMG_Load(filename);
	if (surface == nullptr) {
		std::cerr << "Did you spell the file right?" << std::endl;
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void Entity::ApplyForce(Vec3 force)
{
	acc.x = force.x / mass;
	acc.y = force.y / mass;
	acc.z = force.z / mass;
}

void Entity::Update(float deltaTime)
{
	pos.x += (vel.x * deltaTime + 0.5 * acc.x * std::pow(deltaTime, 2));
	pos.y += (vel.y * deltaTime + 0.5 * acc.y * std::pow(deltaTime, 2));
	pos.z += (vel.z * deltaTime + 0.5 * acc.z * std::pow(deltaTime, 2));

	vel.x += acc.x * deltaTime;
	vel.y += acc.y * deltaTime;
	vel.z += acc.z * deltaTime;
}

void Entity::Collision(Entity& ball) {

	Vec3 BallToBall = Vec3(pos.x - ball.pos.x, pos.y - ball.pos.y, pos.z - ball.pos.z);
	float distance = sqrt(BallToBall.x * BallToBall.x + BallToBall.y * BallToBall.y + BallToBall.z * BallToBall.z);

	if (distance < (r + ball.r)) {
		BallToBall = VMath::normalize(BallToBall);

		pos.print("Ball position");
		ball.pos.print("Other ball position");
		BallToBall.print("Normal");
		std::cout << distance << "\n \n";
		std::cout << "Collision detected: distance = " << distance
			<< ", combined rads = " << (r + ball.r) << std::endl;

		if (distance < VERY_SMALL) {
			std::cerr << "Objects are too close" << std::endl;
			return;
		}

		float overlap = (r + ball.r) - distance;
		pos += BallToBall * (overlap / 2.0f);
		ball.pos -= BallToBall * (overlap / 2.0f);

		Vec3 Velocity = vel - ball.vel;

		float velNormal = VMath::dot(Velocity, BallToBall);

		if (velNormal > 0) {
			return;
		}

		float e = 1.0f;

		float j = -(1 + e) * velNormal;
		j /= ((1 / mass) + (1 / ball.mass));

		Vec3 impulse = BallToBall * j;

		vel += impulse / mass;
		ball.vel -= impulse / ball.mass;
	}
}
	void Entity::MoveLeft(float deltaTime) 
	{
		pos.x -= speed * deltaTime; 
	}

	void Entity::MoveRight(float deltaTime) 
	{
		pos.x += speed * deltaTime; 
	}
	void Entity::CollisionPlatform(Entity& obj) {

		float  width = 6.0f;
		float height = 1.5f;

		float rectHalfWidth = width * 0.5f;
		float rectHalfHeight = height * 0.5f;

		float closestX = std::max(obj.pos.x - rectHalfWidth, std::min(pos.x, obj.pos.x + rectHalfWidth));
		float closestY = std::max(obj.pos.y - rectHalfHeight, std::min(pos.y, obj.pos.y + rectHalfHeight));
		Vec3 closestPoint(closestX, closestY, 0.0f);

		Vec3 distanceVec = pos - closestPoint;
		float distanceSquared = VMath::dot(distanceVec, distanceVec);

		if (distanceSquared < r + r) {
			std::cout << "\n---------------------------------------" << std::endl;
			std::cout << "Ball collided with platform\n";
			std::cout << "Distance Squared: " << distanceSquared << std::endl;
			std::cout << "---------------------------------------" << std::endl;
			Vec3 normal = VMath::normalize(distanceVec);
			vel = VMath::reflect(vel, normal);

			float velNormal = VMath::dot(vel, normal);

			//float overlap = r - std::sqrt(distanceSquared);
			//pos += normal * (overlap + 0.01f);

			float overlap = (r + r) - distanceSquared;
			Vec3 correction = overlap*normal;
			pos += correction;

			float e = 1.0f;

			float j = -(1 + e) * velNormal;
			j /= (1 / mass);

			Vec3 impulse = normal * j;

			vel += impulse / mass;
			distanceSquared = 0.0f;
		}
	}

	 

