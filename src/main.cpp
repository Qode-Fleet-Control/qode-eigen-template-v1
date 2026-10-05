// A small Eigen program that does a few classic linear-algebra jobs and checks
// its own answers. Every step prints what it computed and the residual it
// checked; the exit code is the number of failed checks (0 = all passed), so
// it doubles as the job's test.
//
//   1. solve a dense linear system Ax = b (LU with partial pivoting)
//   2. fit a line to noisy points by least squares (QR)
//   3. eigen-decompose a symmetric matrix (SelfAdjointEigenSolver)
//   4. invert a matrix and check A * A^-1 == I

#include <Eigen/Dense>

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& name, double error, double tolerance)
{
    const bool ok = std::isfinite(error) && error <= tolerance;
    std::cout << (ok ? "  PASS " : "  FAIL ") << name << "  (error " << error << ", tolerance " << tolerance
              << ")\n";
    if (!ok)
        ++failures;
}

void linear_system()
{
    std::cout << "1. Linear system Ax = b\n";
    Eigen::Matrix3d A;
    A << 2, 1, -1,
        -3, -1, 2,
        -2, 1, 2;
    const Eigen::Vector3d b(8, -11, -3);
    const Eigen::Vector3d x = A.partialPivLu().solve(b);
    std::cout << "   x = " << x.transpose() << "\n";
    check("residual |Ax - b|", (A * x - b).norm(), 1e-12);
    check("known solution (2, 3, -1)", (x - Eigen::Vector3d(2, 3, -1)).norm(), 1e-12);
}

void least_squares()
{
    std::cout << "2. Least-squares line fit y = m*x + c\n";
    // Points on y = 1.5x + 0.5 with a small, deterministic wobble.
    constexpr int n = 50;
    Eigen::MatrixXd X(n, 2);
    Eigen::VectorXd y(n);
    for (int i = 0; i < n; ++i) {
        const double xi = i * 0.2;
        X(i, 0) = xi;
        X(i, 1) = 1.0;
        y(i) = 1.5 * xi + 0.5 + 0.01 * std::sin(7.0 * i);
    }
    const Eigen::Vector2d coef = X.colPivHouseholderQr().solve(y);
    std::cout << "   m = " << coef(0) << ", c = " << coef(1) << "\n";
    check("slope close to 1.5", std::abs(coef(0) - 1.5), 1e-2);
    check("intercept close to 0.5", std::abs(coef(1) - 0.5), 1e-2);
}

void eigen_decomposition()
{
    std::cout << "3. Symmetric eigen-decomposition\n";
    Eigen::Matrix3d S;
    S << 4, 1, 0,
         1, 3, 1,
         0, 1, 2;
    const Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(S);
    if (solver.info() != Eigen::Success) {
        check("solver converged", 1.0, 0.0);
        return;
    }
    const auto& values = solver.eigenvalues();
    const auto& vectors = solver.eigenvectors();
    std::cout << "   eigenvalues = " << values.transpose() << "\n";
    const Eigen::Matrix3d rebuilt = vectors * values.asDiagonal() * vectors.transpose();
    check("V * D * V^T == S", (rebuilt - S).norm(), 1e-12);
    check("trace == sum of eigenvalues", std::abs(S.trace() - values.sum()), 1e-12);
}

void inverse()
{
    std::cout << "4. Matrix inverse\n";
    Eigen::Matrix4d M;
    M << 4, 7, 2, 3,
         0, 5, 1, 1,
         3, 0, 6, 2,
         1, 2, 0, 8;
    const Eigen::Matrix4d inv = M.inverse();
    std::cout << "   det = " << M.determinant() << "\n";
    check("M * M^-1 == I", (M * inv - Eigen::Matrix4d::Identity()).norm(), 1e-12);
}

}  // namespace

int main()
{
    std::cout << "Eigen " << EIGEN_WORLD_VERSION << "." << EIGEN_MAJOR_VERSION << "." << EIGEN_MINOR_VERSION
              << "\n";
    linear_system();
    least_squares();
    eigen_decomposition();
    inverse();
    std::cout << (failures == 0 ? "All checks passed.\n" : "Some checks FAILED.\n");
    return failures;
}
