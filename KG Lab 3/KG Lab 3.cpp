#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cmath>
using namespace std;
using namespace sf;
class Matrix 
{
    struct MatrixSize 
    {
        int w;
        int h;
    } size;
    vector<double> vec;
public:
    Matrix(int h, int w, const double* coef) 
    {
        size.w = w;
        size.h = h;
        vec.assign(coef, coef + w * h);
    }
    Matrix(int h, int w, initializer_list<double> coef) 
    {
        size.w = w;
        size.h = h;
        vec.assign(coef.begin(), coef.end());
    }
    static Matrix XY(Vector2f vec) 
    {
        return Matrix(3, 1, { vec.x,vec.y,1 });
    }
    double get(int i, int j) const 
    {
        return vec[size.w * i + j];
    }
    Vector2f toVec() const 
    {
        return Vector2f{ (float)get(0,0),(float)get(1,0) };
    }
    Matrix operator*(const Matrix& a) const
    {
        const Matrix& b = *this;
        if (a.size.w != b.size.h)
            throw runtime_error("MULTIPLICATION IS NOT POSSIBLE");
        int h = a.size.h;
        int w = b.size.w;
        vector<double> v;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++) {
                double sum = 0;
                for (int k = 0; k < a.size.w; k++)
                    sum += a.get(i, k) * b.get(k, j);
                v.push_back(sum);
            }
        return Matrix(h, w, v.data());
    }
    static Matrix translation(double x, double y) 
    {

        return Matrix(3, 3, { 1,0,x,
                           0,1,y,
                           0,0,1 });
    }
    static Matrix rotationAroundOrigin(double phi) 
    {
        return Matrix(3, 3, { cos(phi), sin(phi),0,
                           -sin(phi),  cos(phi),0,
                           0,         0,       1 });
    }
    static Matrix scaleOrigin(double x, double y) 
    {
        return Matrix(3, 3, { x, 0, 0,
                           0, y, 0,
                           0, 0, 1 });
    }
    static Matrix shearHOrigin(double m) 
    {
        return Matrix(3, 3, { 1, m, 0,
                           0, 1, 0,
                           0, 0, 1 });
    }
    static Matrix shearVOrigin(double m) 
    {
        return Matrix(3, 3, { 1, 0, 0,
                           m, 1, 0,
                           0, 0, 1 });
    }
};
class PolygonShape 
{
public:
    VertexArray shp;
    PolygonShape(int n) 
    {
        srand(time(0));
        shp.setPrimitiveType(PrimitiveType::LineStrip);
        shp.clear();
        for (int i = 0; i < n; i++) 
        {
            shp.append(Vertex(Vector2f{ (float)(rand() % 800),(float)(rand() % 600) }, Color::White));
        }
        shp.append(shp[0]);
    }
    PolygonShape(const PolygonShape& p) 
    {
        shp.setPrimitiveType(LineStrip);
        shp.clear();
        for (int i = 0; i < p.shp.getVertexCount(); i++) 
        {
            shp.append(p.shp[i]);
        }
    }
    void transform(const Matrix& a) {
        for (int i = 0; i < shp.getVertexCount(); i++) 
        {
            Vertex& v = shp[i];

            v.position = (Matrix::XY(v.position) * a).toVec();
        }
    }
    void translate(double x, double y) 
    {
        transform(Matrix::translation(x, y));
    }
    void rotateAroundOrigin(double phi) 
    {
        transform(Matrix::rotationAroundOrigin(phi));
    }
    void rotateAroundPoint(double x, double y, double phi) 
    {
        translate(-x, -y);
        rotateAroundOrigin(phi);
        translate(x, y);
    }
    void scaleAroundOrigin(double kx, double ky) 
    {
        transform(Matrix::scaleOrigin(kx, ky));
    }
    void scaleAroundPoint(double x, double y, double kx, double ky) 
    {
        translate(-x, -y);
        scaleAroundOrigin(kx, ky);
        translate(x, y);
    }
    void shearAroundOrigin(double my) 
    {
        transform(Matrix::shearVOrigin(my));
    }
    void shearAroundPoint(double x, double y, double my) 
    {
        translate(-x, -y);
        shearAroundOrigin(my);
        translate(x, y);
    }
    void shearAroundPointWithAxis(double x, double y, double phi, double my) 
    {
        translate(-x, -y);
        rotateAroundOrigin(phi);
        shearAroundOrigin(my);
        rotateAroundOrigin(-phi);
        translate(x, y);
    }
    Vector2f getMassCenter() const 
    {
        float sumx = 0, sumy = 0;
        int vc = shp.getVertexCount();
        for (int i = 0; i < vc; i++) 
        {
            sumx += shp[i].position.x;
            sumy += shp[i].position.y;
        }
        return Vector2f{ sumx / vc,sumy / vc };
    }
};
int main() 
{
    int z;
    cout << "Enter the desired number of polygon vertices:" << endl;
    cin >> z;
    PolygonShape original_shape(z);
    PolygonShape shape(original_shape);
    RenderWindow win(VideoMode(1000, 1000), "AFFIN");
    int anim = 0;
    Clock clk;
    while (win.isOpen()) 
    {
        Event event;
        auto mc = shape.getMassCenter();
        while (win.pollEvent(event)) 
        {

            if (event.type == Event::Closed) 
            {
                win.close();
            }
            if (event.type == Event::KeyPressed) 
            {
                switch (event.key.code) 
                {
                case Keyboard::Left:
                    shape.translate(-10, 0);
                    break;
                case Keyboard::Right:
                    shape.translate(10, 0);
                    break;
                case Keyboard::Up:
                    shape.translate(0, -10);
                    break;
                case Keyboard::Down:
                    shape.translate(0, 10);
                    break;
                case Keyboard::Q:

                    shape.rotateAroundPoint(mc.x, mc.y, M_PI_4);
                    break;
                case Keyboard::E:

                    shape.rotateAroundPoint(mc.x, mc.y, -M_PI_4);
                    break;
                case Keyboard::W:
                    shape.scaleAroundPoint(mc.x, mc.y, 2, 2);
                    break;
                case Keyboard::S:
                    shape.scaleAroundPoint(mc.x, mc.y, 0.5, 0.5);
                    break;
                case Keyboard::D:
                    shape.shearAroundPointWithAxis(mc.x, mc.y, M_PI_4 * 3, M_SQRT2);
                    break;
                case Keyboard::A:
                    shape.shearAroundPointWithAxis(mc.x, mc.y, 3 * M_PI_4, -M_SQRT2);
                    break;
                case Keyboard::Space:
                    anim = (anim + 1) % 5;
                case Keyboard::R:
                    if (event.key.shift)
                        original_shape = PolygonShape(shape);
                    shape = PolygonShape(original_shape);
                    break;
                default:
                    break;
                }
            }
        }
        Time elapsed = clk.restart();
        float dt = elapsed.asSeconds();
        switch (anim) 
        {
        case 1:
            shape.translate(100 * dt * 0.5, -100 * dt);
            break;
        case 2:
            shape.rotateAroundPoint(mc.x, mc.y, M_PI / 10.0 * dt);
            break;
        case 3:
            shape.scaleAroundPoint(mc.x, mc.y, 1 + dt / 10, 1 + dt / 10);
            break;
        case 4:
            shape.shearAroundPointWithAxis(mc.x, mc.y, 3 * M_PI_4, M_SQRT2 * dt);
            break;
        case 0:
        default:
            break;
        }
        win.clear();
        win.draw(shape.shp);
        win.display();
    }
    return 0;
}