#include "camera.h"
#include "math.h"
#include <cmath>

Camera Camera_Create(Vec3 position, Vec3 up, float yaw, float pitch) {
    Camera camera = {};
    camera.front = Vec3{0.0f, 0.0f, -1.0f};
    camera.movementSpeed = SPEED;
    camera.mouseSensitivity = SENSITIVITY;
    camera.zoom = ZOOM;
    camera.position = position;
    camera.worldUp = up;
    camera.yaw = yaw;
    camera.pitch = pitch;

    Camera_UpdateVectors(&camera);

    return camera;
}

Mat4 Camera_GetViewMatrix(Camera *camera) {
    return Mat4_LookAt(camera->position,
                       Vec3_Add(camera->position, camera->front), camera->up);
}

void Camera_ProcessKeyboard(Camera *camera, CameraMovement direction,
                            float deltaTime) {
    float velocity = camera->movementSpeed * deltaTime;

    if (direction == FORWARD) {
        camera->position = Vec3_Add(camera->position,
                                    Vec3_ScalarMult(camera->front, velocity));
    }
    if (direction == BACKWARD) {
        camera->position = Vec3_Subtract(
            camera->position, Vec3_ScalarMult(camera->front, velocity));
    }
    if (direction == LEFT) {
        camera->position = Vec3_Subtract(
            camera->position, Vec3_ScalarMult(camera->right, velocity));
    }
    if (direction == RIGHT) {
        camera->position = Vec3_Add(camera->position,
                                    Vec3_ScalarMult(camera->right, velocity));
    }
    if (direction == UP) {
        camera->position =
            Vec3_Add(camera->position, Vec3_ScalarMult(camera->up, velocity));
    }
    if (direction == DOWN) {
        camera->position = Vec3_Subtract(camera->position,
                                         Vec3_ScalarMult(camera->up, velocity));
    }
}

void Camera_ProcessMouseMovement(Camera *camera, float offsetX, float offsetY,
                                 bool constrainPitch) {
    offsetX *= camera->mouseSensitivity;
    offsetY *= camera->mouseSensitivity;

    camera->yaw += offsetX;
    camera->pitch += offsetY;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (camera->pitch > 89.0f) {
            camera->pitch = 89.0f;
        }
        if (camera->pitch < -89.0f) {
            camera->pitch = -89.0f;
        }
    }

    Camera_UpdateVectors(camera);
}

void Camera_ProcessMouseScroll(Camera *camera, float offsetY) {
    camera->zoom -= (float)offsetY;
    if (camera->zoom < 1.0f) {
        camera->zoom = 1.0f;
    }
    if (camera->zoom > 45.0f) {
        camera->zoom = 45.0f;
    }
}

void Camera_UpdateVectors(Camera *camera) {
    // calculate the new Front vector
    Vec3 front;
    front.x = cos(DegToRadians(camera->yaw)) * cos(DegToRadians(camera->pitch));
    front.y = sin(DegToRadians(camera->pitch));
    front.z = sin(DegToRadians(camera->yaw)) * cos(DegToRadians(camera->pitch));
    camera->front = Vec3_Normalize(front);

    // also re-calculate the Right and Up vector
    // normalize the vectors, because their length gets
    // closer to 0 the more you look up or down which
    // results in slower movement.
    camera->right = Vec3_Normalize(Vec3_Cross(camera->front, camera->worldUp));
    camera->up = Vec3_Normalize(Vec3_Cross(camera->right, camera->front));
}
