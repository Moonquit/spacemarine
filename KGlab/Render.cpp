#include "Render.h"
#include "ObjLoader.h"
#include "Texture.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstring>
#include <cstdio>

#include "debout.h"
#include "MyOGL.h"
extern OpenGL gl;

#include "Light.h"
Light light;

#include "Camera.h"
Camera camera;

ObjModel ship;

Texture stars_tex;
Texture sun_tex;
Texture earth_tex;
Texture moon_tex;
Texture mars_tex;
Texture jupiter_tex;
Texture saturn_tex;
Texture saturn_ring_tex;

double full_time = 0;

double shipX = 0;
double shipY = -22;
double shipZ = 0;
double shipAngle = 0;
double shipSpeed = 0;
double shipPitch = 0;

double cameraX = 0;
double cameraY = 0;
double cameraZ = 0;

bool collision = false;
bool exploding = false;

double explosionTimer = 0;
double explosionX = 0;
double explosionY = 0;
double explosionZ = 0;

int passedAsteroids = 0;

GLuint fontBase = 0;
bool fontReady = false;

const int ASTEROIDS_COUNT = 14;

double asteroidX[ASTEROIDS_COUNT] = {
    -5.0, 4.2, -7.5, 8.0, -3.2, 6.8, -9.0,
    10.5, -4.8, 3.5, -11.0, 7.2, -6.0, 11.5
};

double asteroidY[ASTEROIDS_COUNT] = {
    -13.0, -9.5, -5.0, -1.0, 3.2, 6.5, 9.5,
    12.5, 16.0, 19.0, 22.5, 25.0, 28.5, 31.5
};

double asteroidZ[ASTEROIDS_COUNT] = {
    0.2, -1.0, 1.5, -0.8, 2.0, 0.6, -1.5,
    1.2, -2.0, 0.8, 1.8, -1.2, 0.4, -0.5
};

double asteroidSize[ASTEROIDS_COUNT] = {
    0.9, 1.1, 0.8, 1.2, 0.95, 1.0, 1.25,
    0.85, 1.15, 0.9, 1.2, 1.0, 0.95, 1.1
};

bool asteroidPassed[ASTEROIDS_COUNT] = { false };

void buildFont()
{
    HDC hdc = wglGetCurrentDC();

    HFONT font = CreateFontA(
        18,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE | DEFAULT_PITCH,
        "Consolas"
    );

    SelectObject(hdc, font);

    fontBase = glGenLists(256);
    wglUseFontBitmapsA(hdc, 0, 255, fontBase);

    fontReady = true;
}

void drawTextLine(int x, int y, const char* s)
{
    if (!fontReady)
        return;

    glRasterPos2i(x, y);
    glListBase(fontBase);
    glCallLists((GLsizei)strlen(s), GL_UNSIGNED_BYTE, s);
}

void material(double r, double g, double b, double shine)
{
    float amb[] = { float(r * 0.18), float(g * 0.18), float(b * 0.18), 1.0f };
    float dif[] = { float(r), float(g), float(b), 1.0f };
    float spec[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    float emission[] = { 0, 0, 0, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glMaterialf(GL_FRONT, GL_SHININESS, float(shine));
}

void materialEmission(double r, double g, double b)
{
    float amb[] = { float(r * 0.45), float(g * 0.45), float(b * 0.45), 1.0f };
    float dif[] = { float(r), float(g), float(b), 1.0f };
    float spec[] = { float(r), float(g), float(b), 1.0f };
    float emission[] = { float(r * 0.85), float(g * 0.85), float(b * 0.85), 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glMaterialf(GL_FRONT, GL_SHININESS, 40);
}

void materialShip()
{
    float amb[] = { 0.20f, 0.24f, 0.35f, 1.0f };
    float dif[] = { 0.70f, 0.78f, 1.00f, 1.0f };
    float spec[] = { 0.85f, 0.90f, 1.00f, 1.0f };
    float emission[] = { 0.08f, 0.10f, 0.18f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glMaterialf(GL_FRONT, GL_SHININESS, 55);
}

void resetEmission()
{
    float emission[] = { 0, 0, 0, 1 };
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void drawBox(double sx, double sy, double sz)
{
    double x = sx / 2.0;
    double y = sy / 2.0;
    double z = sz / 2.0;

    glBegin(GL_QUADS);

    glNormal3d(0, 0, 1);
    glVertex3d(-x, -y, z);
    glVertex3d(x, -y, z);
    glVertex3d(x, y, z);
    glVertex3d(-x, y, z);

    glNormal3d(0, 0, -1);
    glVertex3d(x, -y, -z);
    glVertex3d(-x, -y, -z);
    glVertex3d(-x, y, -z);
    glVertex3d(x, y, -z);

    glNormal3d(0, 1, 0);
    glVertex3d(-x, y, -z);
    glVertex3d(-x, y, z);
    glVertex3d(x, y, z);
    glVertex3d(x, y, -z);

    glNormal3d(0, -1, 0);
    glVertex3d(-x, -y, -z);
    glVertex3d(x, -y, -z);
    glVertex3d(x, -y, z);
    glVertex3d(-x, -y, z);

    glNormal3d(1, 0, 0);
    glVertex3d(x, -y, -z);
    glVertex3d(x, y, -z);
    glVertex3d(x, y, z);
    glVertex3d(x, -y, z);

    glNormal3d(-1, 0, 0);
    glVertex3d(-x, -y, -z);
    glVertex3d(-x, -y, z);
    glVertex3d(-x, y, z);
    glVertex3d(-x, y, -z);

    glEnd();
}

void resetShip()
{
    shipX = 0;
    shipY = -22;
    shipZ = 0;
    shipAngle = 0;
    shipSpeed = 0;
    shipPitch = 0;
    collision = false;
}

void startExplosion()
{
    exploding = true;
    explosionTimer = 1.2;

    explosionX = shipX;
    explosionY = shipY;
    explosionZ = shipZ;

    shipSpeed = 0;
}

void setupThirdPersonCamera()
{
    double a = shipAngle * 3.1415926535 / 180.0;

    double fx = -sin(a);
    double fy = cos(a);

    cameraX = shipX - fx * 9.0;
    cameraY = shipY - fy * 9.0;
    cameraZ = shipZ + 4.8;

    double targetX = shipX + fx * 5.0;
    double targetY = shipY + fy * 5.0;
    double targetZ = shipZ + 0.8;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        cameraX, cameraY, cameraZ,
        targetX, targetY, targetZ,
        0, 0, 1
    );
}

void setupSunLight()
{
    float globalAmbient[] = { 0.13f, 0.13f, 0.15f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    float pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float ambient[] = { 0.20f, 0.17f, 0.12f, 1.0f };
    float diffuse[] = { 1.00f, 0.88f, 0.55f, 1.0f };
    float specular[] = { 1.00f, 0.95f, 0.75f, 1.0f };

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.008f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.00055f);
}

void drawSkySphere()
{
    glPushMatrix();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glColor3d(1, 1, 1);

    glTranslated(cameraX, cameraY, cameraZ);

    stars_tex.Bind();

    GLUquadric* q = gluNewQuadric();

    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);
    gluQuadricOrientation(q, GLU_INSIDE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glRotated(90, 1, 0, 0);
    gluSphere(q, 120.0, 64, 32);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    gluDeleteQuadric(q);

    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawOrbit(double r)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3d(0.20, 0.25, 0.40);

    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 260; ++i)
    {
        double t = 2.0 * 3.1415926535 * i / 260.0;
        glVertex3d(cos(t) * r, sin(t) * r, -0.01);
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

void drawTexturedSphere(Texture& tex, double radius, int slices, int stacks)
{
    GLUquadric* q = gluNewQuadric();

    glColor3d(1, 1, 1);

    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);

    glEnable(GL_TEXTURE_2D);
    tex.Bind();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    gluSphere(q, radius, slices, stacks);

    glBindTexture(GL_TEXTURE_2D, 0);
    gluDeleteQuadric(q);
}

void drawTexturedSphereBright(Texture& tex, double radius, int slices, int stacks)
{
    GLUquadric* q = gluNewQuadric();

    glDisable(GL_LIGHTING);
    glColor3d(1, 1, 1);

    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);

    glEnable(GL_TEXTURE_2D);
    tex.Bind();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    gluSphere(q, radius, slices, stacks);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBindTexture(GL_TEXTURE_2D, 0);
    gluDeleteQuadric(q);

    glEnable(GL_LIGHTING);
}

void drawSun()
{
    glPushMatrix();

    materialEmission(1.0, 0.78, 0.25);
    glRotated(full_time * 5.0, 0, 0, 1);
    drawTexturedSphereBright(sun_tex, 3.0, 56, 28);

    resetEmission();

    glPopMatrix();
}

void getPlanetPosition(double orbitR, double speed, double& x, double& y, double& z)
{
    double angle = full_time * speed;

    x = cos(angle) * orbitR;
    y = sin(angle) * orbitR;
    z = 0;
}

void drawPlanet(Texture& tex, double orbitR, double size, double speed, double selfSpeed)
{
    double x, y, z;
    getPlanetPosition(orbitR, speed, x, y, z);

    glPushMatrix();

    glTranslated(x, y, z);
    glRotated(full_time * selfSpeed, 0, 0, 1);

    material(0.95, 0.95, 0.95, 34);
    drawTexturedSphere(tex, size, 48, 24);

    glPopMatrix();
}

void drawMoon(double earthOrbitR, double earthSpeed)
{
    double ex, ey, ez;
    getPlanetPosition(earthOrbitR, earthSpeed, ex, ey, ez);

    double moonAngle = full_time * 2.4;

    double mx = ex + cos(moonAngle) * 1.8;
    double my = ey + sin(moonAngle) * 1.8;

    glPushMatrix();

    glTranslated(mx, my, 0);
    material(0.95, 0.95, 0.95, 18);
    drawTexturedSphere(moon_tex, 0.27, 24, 12);

    glPopMatrix();
}

void drawSaturnRingSide(double innerR, double outerR, double normalZ)
{
    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= 260; ++i)
    {
        double t = 2.0 * 3.1415926535 * i / 260.0;

        double c = cos(t);
        double s = sin(t);

        glNormal3d(0, 0, normalZ);

        glTexCoord2d(0.0, double(i) / 40.0);
        glVertex3d(c * innerR, s * innerR, 0);

        glTexCoord2d(1.0, double(i) / 40.0);
        glVertex3d(c * outerR, s * outerR, 0);
    }

    glEnd();
}

void drawSaturnRing(double innerR, double outerR)
{
    glPushMatrix();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    glColor4d(1.0, 1.0, 1.0, 0.88);

    saturn_ring_tex.Bind();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    drawSaturnRingSide(innerR, outerR, 1.0);
    drawSaturnRingSide(innerR, outerR, -1.0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawSaturn(double orbitR, double size, double speed, double selfSpeed)
{
    double x, y, z;
    getPlanetPosition(orbitR, speed, x, y, z);

    glPushMatrix();

    glTranslated(x, y, z);

    glPushMatrix();
    glRotated(full_time * selfSpeed, 0, 0, 1);
    material(0.78, 0.70, 0.52, 18);
    drawTexturedSphere(saturn_tex, size, 48, 24);
    glPopMatrix();

    glPushMatrix();
    glRotated(25, 1, 0, 0);
    drawSaturnRing(size * 1.35, size * 2.20);
    glPopMatrix();

    glPopMatrix();
}

void drawSolarSystem()
{
    drawOrbit(9.0);
    drawOrbit(14.0);
    drawOrbit(20.0);
    drawOrbit(27.0);

    drawSun();

    drawPlanet(earth_tex, 9.0, 0.90, 0.30, 32.0);
    drawMoon(9.0, 0.30);

    drawPlanet(mars_tex, 14.0, 0.70, 0.21, 26.0);
    drawPlanet(jupiter_tex, 20.0, 1.75, 0.13, 18.0);
    drawSaturn(27.0, 1.45, 0.09, 15.0);
}

void drawAsteroidModel()
{
    glBegin(GL_TRIANGLES);

    glNormal3d(0, 0, 1);
    glVertex3d(0, 0, 0.8);
    glVertex3d(0.9, 0.1, 0);
    glVertex3d(0.2, 0.8, 0);

    glVertex3d(0, 0, 0.8);
    glVertex3d(0.2, 0.8, 0);
    glVertex3d(-0.7, 0.4, 0);

    glVertex3d(0, 0, 0.8);
    glVertex3d(-0.7, 0.4, 0);
    glVertex3d(-0.5, -0.7, 0);

    glVertex3d(0, 0, 0.8);
    glVertex3d(-0.5, -0.7, 0);
    glVertex3d(0.9, 0.1, 0);

    glNormal3d(0, 0, -1);
    glVertex3d(0, 0, -0.7);
    glVertex3d(0.2, 0.8, 0);
    glVertex3d(0.9, 0.1, 0);

    glVertex3d(0, 0, -0.7);
    glVertex3d(-0.7, 0.4, 0);
    glVertex3d(0.2, 0.8, 0);

    glVertex3d(0, 0, -0.7);
    glVertex3d(-0.5, -0.7, 0);
    glVertex3d(-0.7, 0.4, 0);

    glVertex3d(0, 0, -0.7);
    glVertex3d(0.9, 0.1, 0);
    glVertex3d(-0.5, -0.7, 0);

    glEnd();
}

void drawAsteroids()
{
    material(0.38, 0.36, 0.32, 20);

    for (int i = 0; i < ASTEROIDS_COUNT; ++i)
    {
        glPushMatrix();

        glTranslated(asteroidX[i], asteroidY[i], asteroidZ[i]);
        glRotated(full_time * 18.0 + i * 31.0, 1, 1, 0);
        glScaled(asteroidSize[i], asteroidSize[i] * 0.8, asteroidSize[i] * 1.2);

        drawAsteroidModel();

        glPopMatrix();
    }
}

void drawEngineFire()
{
    if (fabs(shipSpeed) < 0.2 || exploding)
        return;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double a = shipAngle * 3.1415926535 / 180.0;

    double fx = -sin(a);
    double fy = cos(a);

    double backX = shipX - fx * 0.55;
    double backY = shipY - fy * 0.55;

    glColor4d(0.2, 0.6, 1.0, 0.55);

    glBegin(GL_TRIANGLES);

    glVertex3d(backX, backY, shipZ);
    glVertex3d(backX - fx * 0.65 + cos(a) * 0.18, backY - fy * 0.65 + sin(a) * 0.18, shipZ + 0.09);
    glVertex3d(backX - fx * 0.65 - cos(a) * 0.18, backY - fy * 0.65 - sin(a) * 0.18, shipZ - 0.09);

    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void drawShip()
{
    if (exploding)
        return;

    materialShip();

    glPushMatrix();

    glTranslated(shipX, shipY, shipZ);
    glRotated(shipAngle, 0, 0, 1);
    glRotated(shipPitch, 1, 0, 0);
    glRotated(90, 1, 0, 0);
    glScaled(0.30, 0.30, 0.30);

    ship.Draw();

    glPopMatrix();

    resetEmission();

    drawEngineFire();
}

void drawExplosion()
{
    if (!exploding)
        return;

    double k = 1.2 - explosionTimer;
    double radius = 0.5 + k * 2.7;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4d(1.0, 0.35, 0.05, 0.65 * explosionTimer);

    GLUquadric* q = gluNewQuadric();

    glPushMatrix();
    glTranslated(explosionX, explosionY, explosionZ);
    gluSphere(q, radius, 24, 12);
    glPopMatrix();

    gluDeleteQuadric(q);

    glColor4d(1.0, 0.9, 0.2, 0.8 * explosionTimer);

    glBegin(GL_LINES);

    for (int i = 0; i < 18; ++i)
    {
        double a = 2.0 * 3.1415926535 * i / 18.0;
        double b = sin(i * 1.7);

        glVertex3d(explosionX, explosionY, explosionZ);
        glVertex3d(
            explosionX + cos(a) * radius * 1.7,
            explosionY + sin(a) * radius * 1.7,
            explosionZ + b * radius
        );
    }

    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void checkCollision()
{
    if (exploding)
        return;

    for (int i = 0; i < ASTEROIDS_COUNT; ++i)
    {
        double dx = shipX - asteroidX[i];
        double dy = shipY - asteroidY[i];
        double dz = shipZ - asteroidZ[i];

        double dist = sqrt(dx * dx + dy * dy + dz * dz);

        if (dist < asteroidSize[i] + 0.35)
        {
            collision = true;
            startExplosion();
            return;
        }

        if (!asteroidPassed[i] && shipY > asteroidY[i])
        {
            asteroidPassed[i] = true;
            passedAsteroids++;
        }
    }

    double px, py, pz;

    double planetData[5][3] = {
        { 0.0, 0.0, 3.0 },
        { 9.0, 0.30, 0.90 },
        { 14.0, 0.21, 0.70 },
        { 20.0, 0.13, 1.75 },
        { 27.0, 0.09, 1.45 }
    };

    for (int i = 0; i < 5; ++i)
    {
        if (i == 0)
        {
            px = 0;
            py = 0;
            pz = 0;
        }
        else
        {
            getPlanetPosition(planetData[i][0], planetData[i][1], px, py, pz);
        }

        double dx = shipX - px;
        double dy = shipY - py;
        double dz = shipZ - pz;

        double dist = sqrt(dx * dx + dy * dy + dz * dz);

        if (dist < planetData[i][2] + 0.35)
        {
            collision = true;
            startExplosion();
            return;
        }
    }
}

void updateShip(double dt)
{
    if (exploding)
    {
        explosionTimer -= dt;

        if (explosionTimer <= 0)
        {
            exploding = false;
            resetShip();
        }

        return;
    }

    collision = false;

    if (gl.isKeyPressed('W'))
        shipSpeed += 4.2 * dt;

    if (gl.isKeyPressed('S'))
        shipSpeed -= 3.0 * dt;

    if (!gl.isKeyPressed('W') && !gl.isKeyPressed('S'))
        shipSpeed *= 0.97;

    if (shipSpeed > 5.0)
        shipSpeed = 5.0;

    if (shipSpeed < -2.0)
        shipSpeed = -2.0;

    if (fabs(shipSpeed) > 0.1)
    {
        double turn = 85.0 * dt;

        if (shipSpeed < 0)
            turn = -turn;

        if (gl.isKeyPressed('A'))
            shipAngle += turn;

        if (gl.isKeyPressed('D'))
            shipAngle -= turn;
    }

    double targetPitch = 0;

    if (gl.isKeyPressed('Q'))
    {
        shipZ += 2.2 * dt;
        targetPitch = -25.0;
    }

    if (gl.isKeyPressed('E'))
    {
        shipZ -= 2.2 * dt;
        targetPitch = 25.0;
    }

    shipPitch += (targetPitch - shipPitch) * 5.0 * dt;

    if (shipZ > 5.0)
        shipZ = 5.0;

    if (shipZ < -5.0)
        shipZ = -5.0;

    double a = shipAngle * 3.1415926535 / 180.0;

    double fx = -sin(a);
    double fy = cos(a);

    shipX += fx * shipSpeed * dt;
    shipY += fy * shipSpeed * dt;

    if (shipX < -34.0) shipX = -34.0;
    if (shipX > 34.0) shipX = 34.0;
    if (shipY < -34.0) shipY = -34.0;
    if (shipY > 34.0) shipY = 34.0;

    checkCollision();
}

void drawInterface()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4d(0.02, 0.02, 0.04, 0.82);

    glBegin(GL_QUADS);
    glVertex2d(8, gl.getHeight() - 8);
    glVertex2d(370, gl.getHeight() - 8);
    glVertex2d(370, gl.getHeight() - 130);
    glVertex2d(8, gl.getHeight() - 130);
    glEnd();

    glColor4d(0.45, 0.65, 1.0, 1.0);

    glBegin(GL_LINE_LOOP);
    glVertex2d(8, gl.getHeight() - 8);
    glVertex2d(370, gl.getHeight() - 8);
    glVertex2d(370, gl.getHeight() - 130);
    glVertex2d(8, gl.getHeight() - 130);
    glEnd();

    glColor3d(1, 1, 1);

    drawTextLine(22, gl.getHeight() - 34, "Course project: Solar system flight");
    drawTextLine(22, gl.getHeight() - 58, "W/S - forward / backward");
    drawTextLine(22, gl.getHeight() - 82, "A/D - turn, Q/E - up / down");
    drawTextLine(22, gl.getHeight() - 106, "Task: fly between asteroids");

    if (exploding)
        drawTextLine(22, gl.getHeight() - 122, "Status: explosion and respawn");
    else
        drawTextLine(22, gl.getHeight() - 122, "Status: flight");

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void initRender()
{
    ship.LoadModel("models/ship6.obj");

    stars_tex.LoadTexture("textures/stars.jpg");
    sun_tex.LoadTexture("textures/sun.jpg");
    earth_tex.LoadTexture("textures/earth.jpg");
    moon_tex.LoadTexture("textures/moon.jpg");
    mars_tex.LoadTexture("textures/mars.jpg");
    jupiter_tex.LoadTexture("textures/jupiter.jpg");
    saturn_tex.LoadTexture("textures/saturn.jpg");
    saturn_ring_tex.LoadTexture("textures/saturn_ring.jpg");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glClearColor(0.002f, 0.003f, 0.010f, 1.0f);

    camera.caclulateCameraPos();

    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);

    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);

    buildFont();
}

void Render(double delta_time)
{
    full_time += delta_time;

    updateShip(delta_time);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    setupThirdPersonCamera();

    setupSunLight();

    glShadeModel(GL_SMOOTH);

    drawSkySphere();
    drawSolarSystem();
    drawAsteroids();
    drawShip();
    drawExplosion();

    drawInterface();
}