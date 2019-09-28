/******************************************************************************/

#include "bed-acc.h"
#include <bigstatsr/prodMatVec.hpp>

/******************************************************************************/

// [[Rcpp::export]]
List bed_stats(Environment obj_bed,
               const IntegerVector& ind_row,
               const IntegerVector& ind_col) {

  XPtr<bed> xp_bed = obj_bed["address"];
  bedAcc macc(xp_bed, ind_row, ind_col);
  size_t n = macc.nrow();
  size_t m = macc.ncol();

  NumericVector sum(m), var(m);
  IntegerVector nb_nona_col(m), nb_nona_row(n, int(m));
  double x, xSum, xxSum;
  int c;

  for (size_t j = 0; j < m; j++) {
    xSum = xxSum = 0;
    c = n;
    for (size_t i = 0; i < n; i++) {
      x = macc(i, j);
      if (x != 3) {
        xSum += x;
        xxSum += x*x;
      } else {
        c--;
        nb_nona_row[i]--;
      }
    }
    sum[j] = xSum;
    var[j] = (xxSum - xSum * xSum / c) / (c - 1);
    nb_nona_col[j] = c;
  }

  int n_bad = Rcpp::sum(2 * nb_nona_col < n);
  if (n_bad > 0) Rcpp::warning("%d variants have >50%% missing values.", n_bad);
  n_bad = Rcpp::sum(2 * nb_nona_row < m);
  if (n_bad > 0) Rcpp::warning("%d samples have >50%% missing values.", n_bad);

  return List::create(_["sum"]  = sum,
                      _["var"]  = var,
                      _["nb_nona_col"] = nb_nona_col,
                      _["nb_nona_row"] = nb_nona_row);
}

/******************************************************************************/

// [[Rcpp::export]]
NumericVector bed_wmean(Environment obj_bed,
                        const IntegerVector& ind_row,
                        const IntegerVector& ind_col,
                        const NumericVector& w) {

  myassert_size(w.size(), ind_row.size());

  XPtr<bed> xp_bed = obj_bed["address"];
  bedAcc macc(xp_bed, ind_row, ind_col);
  size_t n = macc.nrow();
  size_t m = macc.ncol();

  NumericVector wmean(m);
  double x, xwSum, wSum, wSum0 = Rcpp::sum(w);
  int c;

  for (size_t j = 0; j < m; j++) {
    xwSum = 0;
    wSum = wSum0;
    c = n;
    for (size_t i = 0; i < n; i++) {
      x = macc(i, j);
      if (x != 3) {
        xwSum += x * w[i];
      } else {
        c--;
        wSum -= w[i];
      }
    }
    wmean[j] = xwSum / wSum;
  }

  return wmean;
}

/******************************************************************************/

// [[Rcpp::export]]
NumericVector pMatVec4(Environment obj_bed,
                       const IntegerVector& ind_row,
                       const IntegerVector& ind_col,
                       const NumericVector& center,
                       const NumericVector& scale,
                       const NumericVector& x) {

  XPtr<bed> xp_bed = obj_bed["address"];
  bedAccScaled macc(xp_bed, ind_row, ind_col, center, scale);

  return bigstatsr::pMatVec4(macc, x);
}

// [[Rcpp::export]]
NumericVector cpMatVec4(Environment obj_bed,
                        const IntegerVector& ind_row,
                        const IntegerVector& ind_col,
                        const NumericVector& center,
                        const NumericVector& scale,
                        const NumericVector& x) {

  XPtr<bed> xp_bed = obj_bed["address"];
  bedAccScaled macc(xp_bed, ind_row, ind_col, center, scale);

  return bigstatsr::cpMatVec4(macc, x);
}

/******************************************************************************/

// [[Rcpp::export]]
NumericMatrix read_bed_scaled(Environment obj_bed,
                              const IntegerVector& ind_row,
                              const IntegerVector& ind_col,
                              const NumericVector& center,
                              const NumericVector& scale) {

  XPtr<bed> xp_bed = obj_bed["address"];
  bedAccScaled macc_bed(xp_bed, ind_row, ind_col, center, scale);

  size_t n = macc_bed.nrow();
  size_t m = macc_bed.ncol();

  NumericMatrix res(n, m);

  for (size_t j = 0; j < m; j++)
    for (size_t i = 0; i < n; i++)
      res(i, j) = macc_bed(i, j);

  return res;
}

/******************************************************************************/
