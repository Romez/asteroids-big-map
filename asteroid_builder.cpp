#include <cstddef>
#include <cstdio>
#include <iostream>
#include <format>
#include <vector>
#include <raylib.h>
#include <raymath.h>

int screenWidth = 1200;
int screenHeight = 900;

float rotationAngle = 0.001;

const Vector2 screenCenter = Vector2{
	.x = screenWidth / 2.0f,
	.y = screenHeight / 2.0f
};

float handleSize = 40;

Vector2 centerPoint(std::vector<Vector2> points) {
	float x = 0;
	float y = 0;
	float a = 0;

	size_t n = points.size();

	for (size_t i = 0; i < n; i++) {
		float x1 = points[i].x;
		float y1 = points[i].y;

		float x2 = points[(i + 1) % n].x;
		float y2 = points[(i + 1) % n].y;
		
		a += x1 * y2 - x2 * y1;

		float cross = (x1 * y2 - x2 * y1);
		x += (x1 + x2) * cross;
		y += (y1 + y2) * cross;
	}

	x /= (3 * a);
	y /= (3 * a);

	return Vector2 {x, y};
}

class Shape {
public:
	Vector2 polyCenter;
	std::vector<Vector2> points;
	bool isRotating = false;
	ssize_t selected_point = -1;

	Shape(std::vector<Vector2>& points_p) {
		for (Vector2 p : points_p) {
			points.push_back(Vector2Add(screenCenter, p));
		}

		polyCenter = centerPoint(points);
	}

	void toggleRotation() {
		isRotating = !isRotating;
	}

	void appendPoint(Vector2 point) {
		TraceLog(LOG_INFO, std::format("New Point at {}; {}", point.x, point.y).c_str());
		points.push_back(point);
		polyCenter = centerPoint(points);
	}

	void deletePoint(size_t i) {
		TraceLog(LOG_INFO, std::format("Delete Point at {:d}", i).c_str());
		points.erase(points.begin() + i);
		polyCenter = centerPoint(points);
	}

	void rotatePointAt(size_t i) {
        Vector2 p = points[i];
        p = Vector2Subtract(p, polyCenter);
        p = Vector2Rotate(p, rotationAngle);
        p = Vector2Add(p, polyCenter);
        
		points[i] = p;
	}

	ssize_t getPointAtPosition(Vector2 pos) {
		for (size_t i = 0; i < points.size(); i++) {
			Vector2 p = points[i];

			float half = handleSize / 2.0;
			
			if ((p.x - half <= pos.x && pos.x <= p.x + half) && 
				(p.y - half <= pos.y && pos.y <= p.y + half)) {
				return i;
			}
		}

		return -1;
	}

	bool isPointSelected() {
		return selected_point != -1;
	}

	void selectPoint(size_t i) {
		selected_point = i;
	}

	void skipSelectedPoint() {
		selected_point = -1;
	}

	void moveSelectedPoint(Vector2 pos) {
		points[selected_point] = pos;
		polyCenter = centerPoint(points);
	}
};

void drawShape(Shape& shape) {
	for (size_t i = 1; i <= shape.points.size(); i++) {
			Vector2 p1 = shape.points[i-1];
			Vector2 p2 = shape.points[i % shape.points.size()];

			DrawLineV(p1, p2, LIME);
		}

		DrawCircleV(shape.polyCenter, 2, LIME);

		for (size_t i = 0; i < shape.points.size(); i++) {
			Vector2 p = shape.points[i];

			DrawRectangleLines(p.x - (handleSize / 2), p.y - (handleSize / 2), handleSize, handleSize, LIME);
		}
}

void drawPointCoords(Shape& shape) {
    for (size_t i = 0; i < shape.points.size(); i++) {
        int fontSize = 20;

        Vector2 p = Vector2Subtract(screenCenter, shape.points[i]);
        std::string text = std::format("Point {:d} {:.2f} {:.2f}", i, p.x, p.y);

        DrawText(text.c_str(), 0, i * fontSize, fontSize, LIME);
    }
}

int main() {		
	InitWindow(screenWidth, screenHeight, "Polygon");

	std::vector init_points = { 
		Vector2{-80.123, 0},
		Vector2{0, -20.0},
		Vector2{20, 0},
		Vector2{0, 40},
		Vector2{-20, 20},
	};

	Shape shape(init_points);

	while(!WindowShouldClose()) {
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			if (shape.selected_point == -1) {
				Vector2 pos = GetMousePosition();

				ssize_t i = shape.getPointAtPosition(pos);

				if (IsKeyDown(KEY_LEFT_CONTROL)) {
					if (i > -1) {
						shape.deletePoint(i);
					} else {
						shape.appendPoint(pos);
					}
				} else if (i > -1) {
					shape.selectPoint(i);
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (shape.isPointSelected()) {
				shape.skipSelectedPoint();
			}
		}

		if (IsKeyPressed(KEY_SPACE)) {
			shape.toggleRotation();
		}

		if (shape.isPointSelected()) {
			Vector2 pos = GetMousePosition();
			shape.moveSelectedPoint(pos);
		}

		if (shape.isRotating) {
			for (size_t i = 0; i < shape.points.size(); i++) {
				shape.rotatePointAt(i);
			}
		}

		BeginDrawing();

		ClearBackground(DARKBROWN);
		
		drawShape(shape);
        
        drawPointCoords(shape);

		EndDrawing();
	}

	CloseWindow();
	
	return 0;
}
