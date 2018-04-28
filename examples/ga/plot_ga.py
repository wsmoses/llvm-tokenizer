import matplotlib.pyplot as plt
import get_passes
import pickle
import os

def plot_ga(logbook):
    gen = logbook.select("gen")
    fit_mins = logbook.select("min")
    #fit_mins = logbook.chapters["fitness"].select("min")
    #size_avgs = logbook.chapters["size"].select("avg")

    import matplotlib.pyplot as plt

    fig, ax1 = plt.subplots()
    line1 = ax1.plot(gen, fit_mins, "b-", label="Minimum Fitness")
    ax1.set_xlabel("Generation")
    ax1.set_ylabel("Fitness", color="b")
    for tl in ax1.get_yticklabels():
        tl.set_color("b")

    ax2 = ax1.twinx()
  #  line2 = ax2.plot(gen, size_avgs, "r-", label="Average Size")
  #  ax2.set_ylabel("Size", color="r")
  #  for tl in ax2.get_yticklabels():
  #      tl.set_color("r")

  #  lns = line1 + line2
    lns = line1
    labs = [l.get_label() for l in lns]
    ax1.legend(lns, labs, loc="center right")

    plt.show()

if __name__ == "__main__":
    bm_dir = "../../benchmarks/polybench-c-3.2/"
    pgms = get_passes.baseline_11_polybenmarks(bm_dir)
    pgms = pgms[0:1]

    for pgm in pgms:
        print ('Found C source: %s'%pgm)
        filename = os.path.basename(pgm).replace(".c", "") +"_logbook.pkl"
        if os.path.exists(filename):
            logbook = pickle.load( open( filename, "rb" ) )
            plot_ga(logbook)
