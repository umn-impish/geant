'''
you could, for instance, import this file into a Jupyter notebook or other
script for further analyses, and reuse the functions defined here.
'''

import sys
from ast import literal_eval as make_tuple

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.figure
import matplotlib.axes
import numpy as np

#style_file = '../interactive.mplstyle'
#plt.style.use(style_file)
#matplotlib.use('qtagg')


def main():
    output_folders = sys.argv[1:]
    for fold in output_folders:
        # decouple data processing from plotting
        # generally a good practice.
        # then you can reuse the processing stuff elsewhere.
        name = ""
        hg, xe, ye = twod_hist_sipm_positions(fold)
        fig, ax = plot_twod_sipm_histogram(xe, ye, hg, name)
        plt.show()


# type annotations are usually a good idea.
# Python doesn't enforce them but they make the code easier to interpret.
def plot_twod_sipm_histogram(
    xedges: np.ndarray,
    yedges: np.ndarray,
    hg: np.ndarray,
    hist_title: str
) -> tuple[matplotlib.figure.Figure, matplotlib.axes.Axes]:
    fig, ax = plt.subplots(figsize=(10, 4), layout='tight')
    im = ax.pcolormesh(xedges, yedges, hg.transpose())
    ax.set(
        title=hist_title,
        xlabel='x pos (mm)',
        ylabel='y pos (mm)'
    )
    fig.colorbar(im, ax=ax, label='probability')
    return fig, ax


def twod_hist_sipm_positions(fold: str) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    cfg = load_config(f'{fold}/simulation-configuration.tab')
    edges = compute_sipm_bins(cfg)
    dat = load_sipm_hits(f'{fold}/sipm-out.tab')

    # combine all positions into one big list
    combined = [p for d in dat for p in d[1]]
    x, y = [d[0] for d in combined], [d[1] for d in combined]
    return np.histogram2d(x, y, bins=[edges['x'], edges['y']], density=True)


def load_config(cfg_fn: str) -> dict[str, str]:
    ret = dict()
    with open(cfg_fn, 'r') as f:
        for line in f:
            if line[0] == '#' or line == "\n": continue
            k, v = line.split()
            ret[k] = v
    return ret


# helpful type aliases
ThreeDimPositioin = tuple[float, float, float]
SipmHitPair = tuple[int, ThreeDimPositioin]
def load_sipm_hits(fn: str) -> list[SipmHitPair]:
    ret = []
    with open(fn, 'r') as f:
        for line in f:
            num, *positions = line.split()
            n = int(num)
            p = [make_tuple(pos) for pos in positions]
            ret.append((n, p))
    return ret


def compute_sipm_bins(cfg: dict[str, str], bins_per_sipm=20) -> dict[str, np.ndarray]:
    per_row = int(cfg['sipms-per-row'])
    num_rows = int(cfg['num-sipm-rows'])
    space = float(cfg['sipm-spacing'])
    side = float(cfg['sipm-side-length'])

    numbers = {
        'x': per_row,
        'y': num_rows
    }
    sipm_edges = dict()
    nudge = 1e-3

    for k, n in numbers.items():
        # we want to start left (or below) all the SiPMs
        # think about the geometry--this makes sense
        start = (-n * side/2) + ((n - 1) * space / 2)
        sipm_edges[k] = list()
        for i in range(n):
            left = start + i*(side + space)
            right = left + side
            # add the "nudge" to go just beyond edges
            sipm_edges[k].append((left - nudge, right + nudge))

    return {
        k: np.array([
            np.linspace(e[0], e[1], num=bins_per_sipm) for e in edges
        ]).flatten()
        for (k, edges) in sipm_edges.items()
    }


if __name__ == '__main__':
    main()
