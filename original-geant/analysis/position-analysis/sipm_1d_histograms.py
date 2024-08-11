import sys
sys.path.append('analysis/position-analysis/')
import sipm_position_histograms as sph

def main(fold: str, savename: str,plt_title: str):
    cfg = sph.load_config(f'{fold}/simulation-configuration.tab')
    edges = sph.compute_sipm_bins(cfg)
    x_dat = process_oned_data(f'{fold}/sipm-out.tab')
    sipm_sum = calc_tot_sipm_hits(f'{fold}/sipm-out.tab')
    plt_title += " Total Hits: " + sipm_sum

    fig, ax = sph.plt.subplots(figsize=(10, 4), layout='tight')
    ax.set(
        title=plt_title,
        xlabel='x pos (mm)',
        ylabel='probability'
    )
    sph.plt.hist(x_dat,edges['x'], density=True)
    sph.plt.show()
    fig.savefig(savename, dpi=300)

def process_oned_data(fn: str) -> list[float]:
    dat = sph.load_sipm_hits(fn)
    x_dat = list()

    for item in dat:
        for point in item[1]:
            x_dat.append(point[0])
    
    return x_dat

def calc_tot_sipm_hits(fold: str) -> str:
    dat = sph.load_sipm_hits(fold)
    sipm_sum = 0
    for item in dat:
        sipm_sum += item[0]
    return str(sipm_sum)