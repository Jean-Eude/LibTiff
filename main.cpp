#pragma once

// =========================
// Core / base
// =========================
#include <vtkSmartPointer.h>
#include <vtkObject.h>

// =========================
// Data structures
// =========================
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>

// =========================
// Filters
// =========================
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkContourFilter.h>
#include <vtkMarchingCubes.h>

// =========================
// Mapping / rendering (surface)
// =========================
#include <vtkMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>

// =========================
// Actors
// =========================
#include <vtkActor.h>
#include <vtkProperty.h>

// =========================
// Rendering core
// =========================
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// =========================
// Interaction styles
// =========================
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleSwitchBase.h>

// =========================
// Volume rendering (IMPORTANT pour voxel / 3D density)
// =========================
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>

// =========================
// Transfer functions (couleur / transparence)
// =========================
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

// =========================
// Helpers / UI
// =========================
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

// =========================
// Math / utilities (VTK)
# include <vtkMath.h>
#include <random>
#include <cmath>

int main() {
    const int N = 80;

    // =========================
    // 📦 volume 3D (voxels)
    // =========================
    auto volume = vtkSmartPointer<vtkImageData>::New();
    volume->SetDimensions(N, N, N);
    volume->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

    // centre
    const double cx = N / 2.0;
    const double cy = N / 2.0;
    const double cz = N / 2.0;
    const double R = N / 4.0;

    // =========================
    // 🎲 génération voxel
    // =========================
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> noise(0.0, 30.0);

    for (int z = 0; z < N; ++z) {
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {

                auto ptr = static_cast<unsigned char*>(
                    volume->GetScalarPointer(x, y, z)
                );

                // distance au centre
                double dx = x - cx;
                double dy = y - cy;
                double dz = z - cz;
                double d = std::sqrt(dx*dx + dy*dy + dz*dz);

                // =========================
                // 🧠 forme centrale (sphère)
                // =========================
                unsigned char value = 0;

                if (d < R) {
                    // fort signal au centre
                    value = 200;
                } else {
                    // bruit faible autour
                    value = static_cast<unsigned char>(noise(gen));
                }

                *ptr = value;
            }
        }
    }

    // =========================
    // 🎨 transfer fonction couleur
    // =========================
    auto color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
    color->AddRGBPoint(50.0, 0.2, 0.2, 0.8);
    color->AddRGBPoint(200.0, 1.0, 0.3, 0.1);

    auto opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    opacity->AddPoint(0.0, 0.0);
    opacity->AddPoint(100.0, 0.05);
    opacity->AddPoint(200.0, 0.8);

    // =========================
    // 🧊 volume property
    // =========================
    auto property = vtkSmartPointer<vtkVolumeProperty>::New();
    property->SetColor(color);
    property->SetScalarOpacity(opacity);
    property->ShadeOn();

    // =========================
    // 📦 volume mapper (GPU si possible)
    // =========================
    auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    mapper->SetInputData(volume);

    // =========================
    // 📺 volume actor
    // =========================
    auto vol = vtkSmartPointer<vtkVolume>::New();
    vol->SetMapper(mapper);
    vol->SetProperty(property);

    // =========================
    // 🖼 renderer
    // =========================
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddVolume(vol);
    renderer->SetBackground(0.05, 0.05, 0.1);

    auto window = vtkSmartPointer<vtkRenderWindow>::New();
    window->AddRenderer(renderer);
    window->SetSize(900, 700);

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);

    window->Render();
    interactor->Start();

    return 0;
}
// int main() {

//     // =========================
//     // 🎲 données
//     // =========================
//     vtkSmartPointer<vtkPoints> points =
//         vtkSmartPointer<vtkPoints>::New();

//     vtkSmartPointer<vtkDoubleArray> vectors =
//         vtkSmartPointer<vtkDoubleArray>::New();

//     vectors->SetNumberOfComponents(3);
//     vectors->SetName("directions");

//     std::mt19937 gen(std::random_device{}());
//     std::uniform_real_distribution<double> dist_theta(0, 2 * M_PI);
//     std::uniform_real_distribution<double> dist_phi(0, M_PI);
//     std::uniform_real_distribution<double> dist_r(2.0, 8.0);

//     int N = 2000;

//     for (int i = 0; i < N; ++i) {

//         double theta = dist_theta(gen);
//         double phi   = dist_phi(gen);
//         double r     = dist_r(gen);

//         // =========================
//         // 📍 position sphérique
//         // =========================
//         double x = r * sin(phi) * cos(theta);
//         double y = r * sin(phi) * sin(theta);
//         double z = r * cos(phi);

//         points->InsertNextPoint(x, y, z);

//         // =========================
//         // 📌 direction (orientation flèche)
//         // =========================
//         double vx = sin(phi) * cos(theta);
//         double vy = sin(phi) * sin(theta);
//         double vz = cos(phi);

//         vectors->InsertNextTuple3(vx, vy, vz);
//     }

//     // =========================
//     // 📦 polydata
//     // =========================
//     vtkSmartPointer<vtkPolyData> poly =
//         vtkSmartPointer<vtkPolyData>::New();

//     poly->SetPoints(points);
//     poly->GetPointData()->SetVectors(vectors);

//     // =========================
//     // ➡️ flèche de base
//     // =========================
//     vtkSmartPointer<vtkArrowSource> arrow =
//         vtkSmartPointer<vtkArrowSource>::New();

//     // =========================
//     // 📊 glyph (flèches sur chaque point)
//     // =========================
//     vtkSmartPointer<vtkGlyph3DMapper> mapper =
//         vtkSmartPointer<vtkGlyph3DMapper>::New();

//     mapper->SetInputData(poly);
//     mapper->SetSourceConnection(arrow->GetOutputPort());

//     mapper->SetOrientationArray("directions");
//     mapper->SetOrientationModeToDirection();

//     mapper->SetScaleFactor(0.5);
//     mapper->ScalarVisibilityOff();

//     // =========================
//     // 🎨 actor
//     // =========================
//     vtkSmartPointer<vtkActor> actor =
//         vtkSmartPointer<vtkActor>::New();

//     actor->SetMapper(mapper);

//     // =========================
//     // 🖼️ renderer
//     // =========================
//     vtkSmartPointer<vtkRenderer> renderer =
//         vtkSmartPointer<vtkRenderer>::New();

//     renderer->AddActor(actor);
//     renderer->SetBackground(0.05, 0.05, 0.08);

//     // =========================
//     // 🪟 window
//     // =========================
//     vtkSmartPointer<vtkRenderWindow> window =
//         vtkSmartPointer<vtkRenderWindow>::New();

//     window->AddRenderer(renderer);
//     window->SetSize(900, 700);

//     // =========================
//     // 🖱️ interactor
//     // =========================
//     vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//         vtkSmartPointer<vtkRenderWindowInteractor>::New();

//     interactor->SetRenderWindow(window);

//     vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
//         vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

//     interactor->SetInteractorStyle(style);

//     // =========================
//     // 🚀 run
//     // =========================
//     window->Render();
//     interactor->Start();

//     return 0;
// }

// int main() {
//     // =========================
//     // 🎯 SOURCE 3D
//     // =========================
//     auto sphere = vtkSmartPointer<vtkSphereSource>::New();
//     sphere->SetRadius(1.0);
//     sphere->SetThetaResolution(40);
//     sphere->SetPhiResolution(40);

//     // =========================
//     // 🔗 MAPPER
//     // =========================
//     auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//     mapper->SetInputConnection(sphere->GetOutputPort());

//     // =========================
//     // 🎭 ACTOR
//     // =========================
//     auto actor = vtkSmartPointer<vtkActor>::New();
//     actor->SetMapper(mapper);

//     // =========================
//     // 🎥 RENDERER
//     // =========================
//     auto renderer = vtkSmartPointer<vtkRenderer>::New();
//     renderer->AddActor(actor);
//     renderer->SetBackground(0.1, 0.2, 0.3); // bleu foncé

//     // =========================
//     // 🪟 WINDOW
//     // =========================
//     auto window = vtkSmartPointer<vtkRenderWindow>::New();
//     window->AddRenderer(renderer);
//     window->SetSize(800, 600);

//     // =========================
//     // 🖱 INTERACTOR
//     // =========================
//     auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//     interactor->SetRenderWindow(window);

//     auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
//     interactor->SetInteractorStyle(style);

//     // =========================
//     // 🚀 RUN
//     // =========================
//     window->Render();
//     interactor->Start();

//     return 0;
// }












// int main() {
//     using namespace matplot;

//     // 🔥 backend interactif stable
//     setenv("GNUTERM", "x11", 1);

//     std::mt19937 gen(std::random_device{}());
//     std::uniform_real_distribution<double> dist(-5.0, 5.0);

//     int n = 800;

//     std::vector<double> x(n), y(n), z(n);

//     for (int i = 0; i < n; ++i) {
//         x[i] = dist(gen);
//         y[i] = dist(gen);
//         z[i] = x[i]*x[i] + y[i]*y[i]; // paraboloïde
//     }

//     figure();
//     scatter3(x, y, z);

//     title("3D interactif - tourne avec la souris");
//     xlabel("X");
//     ylabel("Y");
//     zlabel("Z");

//     show();   // 👈 fenêtre interactive

//     return 0;
// }





// int main() {
//     using namespace matplot;

//     // 🔥 backend interactif (important)
//     setenv("GNUTERM", "xt", 1);

//     std::mt19937 gen(std::random_device{}());
//     std::uniform_real_distribution<double> dist(-5.0, 5.0);

//     int n = 1000;

//     std::vector<double> x(n), y(n), z(n);

//     for (int i = 0; i < n; ++i) {
//         x[i] = dist(gen);
//         y[i] = dist(gen);
//         z[i] = x[i]*x[i] + y[i]*y[i];
//     }

//     figure();

//     scatter3(x, y, z);

//     title("3D interactif (rotation souris)");
//     xlabel("X");
//     ylabel("Y");
//     zlabel("Z");

//     show();   // 👈 IMPORTANT (fenêtre interactive)

//     return 0;
// }


// int main() {
//     using namespace matplot;

//     // 🔥 backend safe (PNG uniquement → pas de Qt/X11)
//     setenv("GNUTERM", "pngcairo", 1);

//     // =========================
//     // 🎲 génération points 3D
//     // =========================
//     std::vector<std::array<double, 3>> points;

//     std::mt19937 gen(std::random_device{}());
//     std::uniform_real_distribution<double> dist(-5.0, 5.0);

//     int n = 500;

//     for (int i = 0; i < n; ++i) {
//         double x = dist(gen);
//         double y = dist(gen);
//         double z = x*x + y*y;

//         points.push_back({x, y, z});
//     }

//     // =========================
//     // 📦 séparation X Y Z
//     // =========================
//     std::vector<double> X, Y, Z;

//     for (auto &p : points) {
//         X.push_back(p[0]);
//         Y.push_back(p[1]);
//         Z.push_back(p[2]);
//     }

//     // =========================
//     // 📊 FIGURE 1 : 3D
//     // =========================
//     figure();
//     scatter3(X, Y, Z);
//     title("Nuage 3D : z = x^2 + y^2");
//     xlabel("X");
//     ylabel("Y");
//     zlabel("Z");

//     save("plot3d.png");

//     // =========================
//     // 📊 FIGURE 2 : projection 2D (XY)
//     // =========================
//     figure();
//     scatter(X, Y);
//     title("Projection 2D (X,Y)");
//     xlabel("X");
//     ylabel("Y");

//     save("plot2d_xy.png");

//     // =========================
//     // 📊 FIGURE 3 : profil 2D (Z en fonction de X)
//     // =========================
//     figure();
//     plot(X, Z);
//     title("Profil 2D : Z = f(X)");
//     xlabel("X");
//     ylabel("Z");

//     save("profil_xz.png");

//     return 0;
// }

// int main() {
//     using namespace matplot;

//     auto f = figure(true); // invisible

//     std::vector<double> x = {1,2,3,4};
//     std::vector<double> y = {1,4,9,16};

//     plot(x, y);
//     save("plot.png");
// }