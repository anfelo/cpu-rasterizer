#ifndef CAMERA_H_
#define CAMERA_H_

#include "math.h"

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

struct Camera {
    // camera Attributes
    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right;
    Vec3 worldUp;
    // euler Angles
    float yaw;
    float pitch;
    // camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
};

Camera Camera_Create(Vec3 position, Vec3 up, float yaw, float pitch);
Mat4 Camera_GetViewMatrix(Camera *camera);
// processes input received from any keyboard-like input system. Accepts input
// parameter in the form of camera defined ENUM (to abstract it from windowing
// systems)
void Camera_ProcessKeyboard(Camera *camera, CameraMovement direction,
                            float deltaTime);
// processes input received from a mouse input system. Expects the offset
// value in both the x and y direction.
void Camera_ProcessMouseMovement(Camera *camera, float offsetX, float offsetY,
                                 bool constrainPitch);
void Camera_ProcessMouseScroll(Camera *camera, float offsetY);
void Camera_UpdateVectors(Camera *camera);
#endif
