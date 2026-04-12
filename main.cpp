#include <matplot/matplot.h>
#include <vector>
#include <array>
#include <random>























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