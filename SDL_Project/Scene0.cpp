#include "Scene0.h"
#include <SDL.h>
#include <SDL_image.h>
#include <MMath.h>
#include <iostream>
#include "Entity.h"

Scene0::Scene0(SDL_Window* sdlWindow_) {
    window = sdlWindow_;
    renderer = nullptr;
    platform = nullptr;
    xAxis = 30.0f;
    yAxis = 30.0f;
    IMG_Init(IMG_INIT_PNG);
}

Scene0::~Scene0() {
    IMG_Quit();
}

bool Scene0::OnCreate() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    Matrix4 ndc = MMath::viewportNDC(w, h);
    Matrix4 ortho = MMath::orthographic(0.0f, xAxis, 0.0f, yAxis, -1.0f, 1.0f);
    projectionMatrix = ndc * ortho;
    invProjectionMatrix = MMath::inverse(projectionMatrix);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    platform = new Entity();
    platform->pos = Vec3(14.5f, 1.0f, 0.0f);
    platform->SetImage("platform.png", renderer);
    platform->r = 1.0f;
    return true;
}

void Scene0::OnDestroy() {
    for (int i = 0; i < balls.size(); i++) {
        delete balls[i]; 
        balls[i] = nullptr; 
    }
    balls.clear();

    delete platform;
    platform = nullptr;
}

void Scene0::HandleEvents(const SDL_Event& event, const float deltaTime) {
    switch (event.type) {
    case SDL_KEYDOWN:
        if (event.key.keysym.scancode == SDL_SCANCODE_A) {
            keyAdown = true;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_D) {
            keyDdown = true;
        }
        break;
    case SDL_KEYUP:
        if (event.key.keysym.scancode == SDL_SCANCODE_A) {
            keyAdown = false;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_D) {
            keyDdown = false;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:

        if (event.button.button == SDL_BUTTON_LEFT) {
            leftMouseDown = true;
            previousMousePosition = currentMousePosition;
            currentMousePosition =
                Vec3(static_cast<float>(event.motion.x),
                    static_cast<float>(event.motion.y), 0.0f);

            Entity* NewBall = new Entity;
            NewBall->pos = invProjectionMatrix * currentMousePosition;
            NewBall->SetImage("Circle.png", renderer);
            NewBall->mass = 2.0f;
            NewBall->r = 1.5f;
            balls.push_back(NewBall);
            return;
        }
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightMouseDown = true;
        }
        break;

    case SDL_MOUSEMOTION:
        if (rightMouseDown) {
            previousMousePosition = currentMousePosition;
            currentMousePosition =
                Vec3(static_cast<float>(event.motion.x),
                    static_cast<float>(event.motion.y), 0.0f);

            Entity* ThisBall = new Entity;
            if (!balls.empty()) {
                balls.back()->pos = invProjectionMatrix * currentMousePosition;
            }
        }
        break;

    case SDL_MOUSEBUTTONUP:

        if (event.button.button == SDL_BUTTON_LEFT) {
            leftMouseDown = false;

            if (!balls.empty()) {
                balls.back()->pos = invProjectionMatrix * currentMousePosition;
            }
        }

        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightMouseDown = false;

            Vec3 pos1 = invProjectionMatrix * currentMousePosition;
            Vec3 pos2 = invProjectionMatrix * previousMousePosition;

            if (!balls.empty()) {
                balls.back()->vel = (pos1 - pos2) / deltaTime;
            }
        }
        break;
    }
}

void Scene0::Update(const float deltaTime) {
    if (!leftMouseDown) {
        Vec3 gravAccel(0.0f, -9.8f, 0.0f);
        if (deltaTime < VERY_SMALL) return;

        for (size_t i = 0; i < balls.size(); ++i) {
            Entity* ball = balls[i];
            if (!ball) continue;

            Vec3 gravForce = ball->mass * gravAccel;
            ball->ApplyForce(gravForce);
            ball->Update(deltaTime);

            for (size_t j = i + 1; j < balls.size(); ++j) {
                Entity* otherBall = balls[j];
                if (!otherBall) continue;
                ball->Collision(*otherBall);
            }
           ball->CollisionPlatform(*platform);

            if (ball->pos.x - ball->r < 0.0f) {
                ball->pos.x = ball->r;
                ball->vel.x *= -1.0f;
            }
            else if (ball->pos.x + ball->r > xAxis) {
                ball->pos.x = xAxis - ball->r;
                ball->vel.x *= -1.0f;
            }
            if (ball->pos.y - ball->r < 0.0f) {
                ball->pos.y = ball->r;
                ball->vel.y *= -1.0f;
            }
            else if (ball->pos.y + ball->r > yAxis) {
                ball->pos.y = yAxis - ball->r;
                ball->vel.y *= -1.0f;
            }
            ball->vel *= 0.98f;

              
        }
    }
    if (platform->pos.x < 3.0f) {
        keyAdown = false;
    }
    else if (platform->pos.x > xAxis) {
        platform->pos.x = xAxis - 3.0f;
        keyDdown = false;
        return;
    }
    if (keyAdown) {
        platform->MoveLeft(deltaTime);
    }
    if (keyDdown) {
        platform->MoveRight(deltaTime);
    }
    for (size_t i = 0; i < balls.size(); ++i) {
        Entity* ball = balls[i];
        if (!ball) continue;
    }
}

void Scene0::Render() const {
    SDL_RenderClear(renderer);
    for (Entity* ball : balls) {
        if (!ball) continue;
        Vec3 screenCoords = projectionMatrix * ball->pos;
        SDL_Rect square;
        square.x = static_cast<int>(screenCoords.x);
        square.y = static_cast<int>(screenCoords.y);
        square.w = ball->GetSurface()->w;
        square.h = ball->GetSurface()->h;
        square.x -= square.w * 0.5f;
        square.y -= square.h * 0.5f;
        SDL_RenderCopyEx(renderer, ball->GetTexture(), nullptr, &square, ball->angleDeg, nullptr, SDL_FLIP_NONE);
    }

    Vec3 screenCoords = projectionMatrix * platform->pos;
    SDL_Rect square;
    square.x = static_cast<int>(screenCoords.x);
    square.y = static_cast<int>(screenCoords.y);
    square.w = platform->GetSurface()->w;
    square.h = platform->GetSurface()->h;
    square.x -= square.w * 0.5f;
    square.y -= square.h * 0.5f;
    SDL_RenderCopyEx(renderer, platform->GetTexture(), nullptr, &square, platform->angleDeg, nullptr, SDL_FLIP_NONE);
   
    SDL_RenderPresent(renderer);
}