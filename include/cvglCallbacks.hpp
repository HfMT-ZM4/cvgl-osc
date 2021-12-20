#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

class cvglContext;

void processInput(GLFWwindow *window);
void global_resize_callback( GLFWwindow* win, int w, int h);

void logWindow(GLFWwindow* win, cvglContext* context);
