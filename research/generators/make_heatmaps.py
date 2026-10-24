#!/usr/bin/env python3
"""Прототипы тепловых карт по 54 измерениям.

Данные берутся из research/generators/plot_histogram_{d}_{r}_{n}.py
(DEFAULT_TIMES_MS: StdSet, SortedVector, AVL, BTree).

Варианты:
  1 -- шесть слоёв-поверхностей: x=N, y=запросы/вставки, высота внутри слоя
       нормирована, слои стоят на уровнях 1..6 (распределения);
  2 -- x=N, y=запросы/вставки, z=распределение 1..6, цвет=время;
  3 -- 2D heatmap: строки -- распределения, столбцы -- N x отношение;
  4 -- шесть отдельных 3D-панелей: x=N, y=отношение, z=время.

Запуск:
  python3 research/generators/make_heatmaps.py --variant 1 --structure StdSet
  python3 research/generators/make_heatmaps.py --all --structure StdSet
"""

import argparse
import glob
import os
import re

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import cm, colors
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

SHORT = {
    1: "возр.",
    2: "возр. перем.",
    3: "убыв.",
    4: "убыв. перем.",
    5: "случ.",
    6: "случ. перем.",
}
ZSHORT = {1: "возр.", 2: "возр.п.", 3: "убыв.",
          4: "убыв.п.", 5: "случ.", 6: "случ.п."}
ZFULL = {
    1: "1. возрастающая\n(ключи→запросы)",
    2: "2. возрастающая\n(перемешано)",
    3: "3. убывающая\n(ключи→запросы)",
    4: "4. убывающая\n(перемешано)",
    5: "5. случайная\n(ключи→запросы)",
    6: "6. случайная\n(перемешано)",
}
STRUCTS = ["StdSet", "SortedVector", "AVL", "BTree"]
NICE = {"StdSet": "StdSet", "SortedVector": "SortedVector",
        "AVL": "AVL-tree", "BTree": "B-tree"}
N_LABELS = ["100", "$10^4$", "$10^6$"]
RATIO_LABELS = ["0.3", "0.7", "1.0"]

CMAP = plt.get_cmap("jet")


def load_times():
    data = {}
    pattern = re.compile(r"plot_histogram_(\d)_(\d)_(\d)\.py$")
    for path in glob.glob("research/generators/plot_histogram_*_*_*.py"):
        m = pattern.search(path)
        if not m:
            continue
        d, r, n = map(int, m.groups())
        s = open(path, encoding="utf-8").read()
        vals = re.search(r"DEFAULT_TIMES_MS = \[([^\]]+)\]", s).group(1)
        data[(d, r, n)] = [float(x) for x in vals.split(",")]
    return data


def grid_for(data, si):
    """z[d-1, r-1, n-1] = log10(время, ms)."""
    z = np.zeros((6, 3, 3))
    for d in range(1, 7):
        for r in range(1, 4):
            for n in range(1, 4):
                z[d - 1, r - 1, n - 1] = np.log10(data[(d, r, n)][si])
    return z


def _gauss_kernel(sigma):
    size = int(4 * sigma + 1) | 1
    x = np.arange(size) - size // 2
    k = np.exp(-x * x / (2.0 * sigma * sigma))
    return k / k.sum()


def _blur(a, k):
    pad = len(k) // 2
    ap = np.pad(a, pad, mode="edge")
    ap = np.apply_along_axis(lambda m: np.convolve(m, k, mode="valid"), 0, ap)
    ap = np.apply_along_axis(lambda m: np.convolve(m, k, mode="valid"), 1, ap)
    return ap


def smooth2d(a, nx, ny, sigma):
    """Билинейное растяжение сетки + гауссово сглаживание."""
    ny0, nx0 = a.shape
    xs = np.arange(nx0)
    ys = np.arange(ny0)
    xf = np.linspace(0, nx0 - 1, nx)
    yf = np.linspace(0, ny0 - 1, ny)
    tmp = np.array([np.interp(xf, xs, row) for row in a])
    fine = np.array([np.interp(yf, ys, tmp[:, j]) for j in range(nx)]).T
    if sigma > 0:
        fine = _blur(fine, _gauss_kernel(sigma))
    return fine


def log_ticks(z):
    lo = int(np.ceil(z.min()))
    hi = int(np.floor(z.max()))
    ticks = np.arange(lo, hi + 1)
    labels = [f"$10^{{{t}}}$" for t in ticks]
    return ticks, labels


def add_colorbar(fig, ax, norm, label):
    m = cm.ScalarMappable(norm=norm, cmap=CMAP)
    m.set_array([])
    cb = fig.colorbar(m, ax=ax, shrink=0.62, pad=0.10)
    ticks, labels = log_ticks(np.array([norm.vmin, norm.vmax]))
    cb.set_ticks(ticks)
    cb.set_ticklabels(labels)
    cb.set_label(label)
    return cb


def style_xy(ax, fontsize=9):
    ax.set_xticks([0, 1, 2])
    ax.set_xticklabels(N_LABELS, fontsize=fontsize)
    ax.set_yticks([0, 1, 2])
    ax.set_yticklabels(RATIO_LABELS, fontsize=fontsize)
    ax.set_xlabel("N")
    ax.set_ylabel("запросы / вставки")


def style_z_dist(ax, fontsize=6.5):
    ax.set_zticks(range(1, 7))
    ax.set_zticklabels([ZFULL[d] for d in range(1, 7)],
                       fontsize=fontsize, linespacing=0.95)
    for lbl in ax.get_zticklabels():
        lbl.set_bbox(dict(facecolor="white", alpha=0.78, edgecolor="none",
                          pad=0.4))
    ax.set_zlabel("")


def variant1(z, title, out, norm=None):
    fig = plt.figure(figsize=(8, 7))
    ax = fig.add_axes([0.0, 0.0, 0.82, 0.94], projection="3d")
    xs = np.linspace(0, 2, 25)
    ys = np.linspace(0, 2, 25)
    X, Y = np.meshgrid(xs, ys)
    if norm is None:
        norm = colors.Normalize(vmin=z.min(), vmax=z.max())
    vmin, vmax = norm.vmin, norm.vmax

    for d in range(1, 7):
        fine = smooth2d(z[d - 1], 25, 25, sigma=1.0)
        h = (fine - vmin) / (vmax - vmin + 1e-12)
        Z = d + 0.9 * h
        ax.plot_surface(X, Y, Z, facecolors=CMAP(norm(fine)),
                        rstride=1, cstride=1, linewidth=0.25,
                        edgecolor="k", antialiased=True, shade=False,
                        alpha=0.96)

    ax.set_zlim(0.4, 7.1)
    style_xy(ax)
    style_z_dist(ax)
    ax.view_init(elev=24, azim=-60)
    ax.set_box_aspect((1, 1, 0.95), zoom=1.18)

    m = cm.ScalarMappable(norm=norm, cmap=CMAP)
    m.set_array([])
    cax = fig.add_axes([0.905, 0.10, 0.022, 0.80])
    cb = fig.colorbar(m, cax=cax)
    ticks, labels = log_ticks(np.array([norm.vmin, norm.vmax]))
    cb.set_ticks(ticks)
    cb.set_ticklabels(labels)
    cb.set_label("Время, ms (лог. шкала)")
    fig.savefig(out, dpi=200, bbox_inches="tight", pad_inches=0.02)
    plt.close(fig)


def variant2(z, title, out, norm=None):
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection="3d")
    xs = np.linspace(0, 2, 25)
    ys = np.linspace(0, 2, 25)
    X, Y = np.meshgrid(xs, ys)
    if norm is None:
        norm = colors.Normalize(vmin=z.min(), vmax=z.max())

    for d in range(1, 7):
        fine = smooth2d(z[d - 1], 25, 25, sigma=1.0)
        Z = np.full_like(X, d)
        ax.plot_surface(X, Y, Z, facecolors=CMAP(norm(fine)),
                        rstride=1, cstride=1, linewidth=0.3,
                        edgecolor="k", antialiased=True, shade=False,
                        alpha=0.92)

    ax.set_zlim(0.5, 6.6)
    style_xy(ax)
    style_z_dist(ax)
    ax.set_title(title + "\nz = распределение, цвет = время", fontsize=12)
    ax.view_init(elev=25, azim=-60)
    add_colorbar(fig, ax, norm, "Время, ms (лог. шкала)")
    fig.subplots_adjust(left=0.02, right=0.88, top=0.90, bottom=0.03)
    fig.savefig(out, dpi=200)
    plt.close(fig)


def variant3(z, title, out, norm=None):
    fig, ax = plt.subplots(figsize=(11, 6))
    M = np.array([[z[d - 1, r, n] for n in range(3) for r in range(3)]
                  for d in range(6)])
    if norm is None:
        norm = colors.Normalize(vmin=z.min(), vmax=z.max())
    ax.imshow(M, aspect="auto", cmap=CMAP, norm=norm,
              interpolation="bicubic")

    ax.set_yticks(range(6))
    ax.set_yticklabels([f"{d}. {SHORT[d]}" for d in range(1, 7)], fontsize=9)
    ax.set_xticks(range(9))
    ax.set_xticklabels(RATIO_LABELS * 3, fontsize=9)
    ax.set_xlabel("отношение запросов/вставок")
    for x in (2.5, 5.5):
        ax.axvline(x, color="white", linewidth=2)
    for x, lab in ((1, "N = 100"), (4, "N = $10^4$"), (7, "N = $10^6$")):
        ax.text(x, -0.85, lab, ha="center", va="bottom", fontsize=10)
    ax.set_ylim(5.5, -1.3)
    ax.set_title(title + "\n2D heatmap: строки — распределения", fontsize=12)

    m = cm.ScalarMappable(norm=norm, cmap=CMAP)
    m.set_array([])
    cb = fig.colorbar(m, ax=ax, shrink=0.85, pad=0.02)
    ticks, labels = log_ticks(np.array([norm.vmin, norm.vmax]))
    cb.set_ticks(ticks)
    cb.set_ticklabels(labels)
    cb.set_label("Время, ms (лог. шкала)")
    fig.tight_layout()
    fig.savefig(out, dpi=200)
    plt.close(fig)


def variant4(z, title, out, norm=None):
    fig, axes = plt.subplots(2, 3, figsize=(16, 9),
                             subplot_kw={"projection": "3d"})
    xs = np.linspace(0, 2, 21)
    ys = np.linspace(0, 2, 21)
    X, Y = np.meshgrid(xs, ys)
    if norm is None:
        norm = colors.Normalize(vmin=z.min(), vmax=z.max())
    ticks, labels = log_ticks(np.array([norm.vmin, norm.vmax]))

    for d in range(1, 7):
        ax = axes.flat[d - 1]
        fine = smooth2d(z[d - 1], 21, 21, sigma=0.9)
        ax.plot_surface(X, Y, fine, facecolors=CMAP(norm(fine)),
                        rstride=1, cstride=1, linewidth=0.25,
                        edgecolor="k", antialiased=True, shade=False)
        ax.set_zlim(z.min(), z.max())
        ax.set_zticks(ticks)
        ax.set_zticklabels(labels, fontsize=6)
        ax.set_title(f"{d}. {SHORT[d]}", fontsize=10)
        style_xy(ax, fontsize=7)
        ax.view_init(elev=24, azim=-60)

    m = cm.ScalarMappable(norm=norm, cmap=CMAP)
    m.set_array([])
    cax = fig.add_axes([0.915, 0.18, 0.012, 0.64])
    cb = fig.colorbar(m, cax=cax)
    cb.set_ticks(ticks)
    cb.set_ticklabels(labels)
    cb.set_label("Время, ms (лог. шкала)")
    fig.suptitle(title + "\nшесть отдельных 3D-панелей", fontsize=14)
    fig.text(0.012, 0.5, "log10(время, ms)", rotation=90,
             va="center", fontsize=9)
    fig.subplots_adjust(left=0.04, right=0.90, top=0.88, bottom=0.03,
                        wspace=0.05, hspace=0.12)
    fig.savefig(out, dpi=200)
    plt.close(fig)


VARIANTS = {1: variant1, 2: variant2, 3: variant3, 4: variant4}


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--variant", type=int, choices=[1, 2, 3, 4])
    p.add_argument("--all", action="store_true")
    p.add_argument("--structure", default="StdSet", choices=STRUCTS)
    p.add_argument("--suffix", default="v")
    p.add_argument("--out", default=None)
    args = p.parse_args()

    data = load_times()
    si = STRUCTS.index(args.structure)
    z = grid_for(data, si)
    title = NICE[args.structure]
    variants = [1, 2, 3, 4] if args.all else [args.variant]

    # общая цветовая шкала для всех структур
    all_z = [grid_for(data, i) for i in range(len(STRUCTS))]
    gmin = min(zz.min() for zz in all_z)
    gmax = max(zz.max() for zz in all_z)
    gnorm = colors.Normalize(vmin=gmin, vmax=gmax)

    os.makedirs(os.path.join("research", "pictures"), exist_ok=True)
    for v in variants:
        if args.out:
            out = args.out
        else:
            out = os.path.join("research", "pictures",
                               f"heatmap_{args.suffix}{v}.png")
        VARIANTS[v](z, title, out, gnorm)
        print("saved", out)


if __name__ == "__main__":
    main()
