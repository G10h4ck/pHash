/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    D Grant Starkweather - dstarkweather@phash.org

*/

#include "ph_fft.h"

#include <vector>

static void fft_calc(const int N, const double *x, std::complex<double> *X,
                     std::complex<double> *P, const int step,
                     const std::complex<double> *twids) {
    std::complex<double> *S = P + N / 2;
    if (N == 1) {
        X[0] = x[0];
        return;
    }

    fft_calc(N / 2, x, S, X, 2 * step, twids);
    fft_calc(N / 2, x + step, P, X, 2 * step, twids);

    for (int k = 0; k < N / 2; k++) {
        P[k] = P[k] * twids[k * step];
        X[k] = S[k] + P[k];
        X[k + N / 2] = S[k] - P[k];
    }
}

int fft(double *x, int N, std::complex<double> *X) {
    if (x == NULL || X == NULL || N <= 0) return -1;
    // Cooley-Tukey radix-2 requires N to be a power of 2.
    if ((N & (N - 1)) != 0) return -1;

    std::vector<std::complex<double>> twiddle_factors(N / 2);
    std::vector<std::complex<double>> Xt(N);

    for (int k = 0; k < N / 2; k++) {
        twiddle_factors[k] = std::polar(1.0, 2.0 * M_PI * k / N);
    }
    fft_calc(N, x, X, Xt.data(), 1, twiddle_factors.data());

    return 0;
}
