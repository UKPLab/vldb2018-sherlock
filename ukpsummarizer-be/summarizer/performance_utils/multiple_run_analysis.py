import pandas as pd
from combine_funcs import plot_outliers
import matplotlib.pyplot as plt


def get_mse_values(base, rel):
    y = []
    for base_val, rel_val in zip(base, rel):
        diff = (rel_val - base_val) / base_val
        mse = diff**2
        if mse != 0:
            mse *= (diff / abs(diff))
        y.append(mse)
    return y


def get_deviation_metric(values):
    d = {'improvement': [], 'decline': []}

    for val in values:
        if val > 0:
            d['improvement'].append(val)
        elif val < 0:
            d['decline'].append(-1 * val)

    return d


def heartbeat_plot(base_path, compare_path):
    base_data = pd.read_csv(base_path)
    ranking_data = pd.read_csv(compare_path)

    for k in [.1, .2, .3, .4, .5, .6, .7, .8, .9]:
        x = list(base_data.columns.values)[1:]

        base_mean = base_data.mean().tolist()
        rel_mean = ranking_data.loc[ranking_data['k'] == k].mean().tolist()[:-1]

        rel_max = ranking_data.loc[ranking_data['k'] == k].max().tolist()[1:-1]
        rel_min = ranking_data.loc[ranking_data['k'] == k].min().tolist()[1:-1]

        plot_outliers("DUC06", k, x, get_mse_values(base_mean, rel_mean))
        plot_outliers("DUC06", k, x, get_mse_values(base_mean, rel_min), 'orange')
        plot_outliers("DUC06", k, x, get_mse_values(base_mean, rel_max), 'green')

        plt.clf()


def avg_r2(all_path):
    df = pd.read_csv(all_path)

    # Average Rouge Scores Relative k
    print(df.groupby(['k']).mean().T.mean())


def mse_values(base_path, compare_path):
    base_data = pd.read_csv(base_path)
    ranking_data = pd.read_csv(compare_path)

    for k in [.1, .2, .3, .4, .5, .6, .7, .8, .9]:
        base_mean = base_data.mean().tolist()
        rel_mean = ranking_data.loc[ranking_data['k'] == k].mean().tolist()[:-1]

        # rel_max = ranking_data.loc[ranking_data['k'] == k].max().tolist()[1:-1]
        # rel_min = ranking_data.loc[ranking_data['k'] == k].min().tolist()[1:-1]

        mse = get_deviation_metric(get_mse_values(base_mean, rel_mean))
        print(k)
        for label, values in sorted(mse.items(), reverse=True, key=lambda x: x[0]):
            print(label, "%.4f" % (sum(values) / len(values)))
        print("--")


def get_average_num_sents(thr):
    path = '/home/orkan/Dropbox/sherlock/Redundancy sweep/thr {}/iterations.csv'.format(thr)
    df = pd.read_csv(path)
    print(df['sents'].mean())


def plot_iteration_curve(x, y, y_min, y_max, k, topic, fig, c_i):
    colors = ['#ffad01', '#042e60', '#11875d', '#7a5901', '#f7022a', '#29e8e8',
              '#ff7fa7', '#800ed1', '#51b73b', '#730039', '#63b365']

    plt.figure(2)

    plt.xlabel('Iteration t')
    plt.xlabel('Zeit in s')
    plt.ylabel('Qualität in ROUGE-2')
    plt.grid(True, which="both")
    plt.title("R2-Wert pro Zeit für Topic X Summary:" + topic)

    t = []
    t0 = 0.0
    for xi in x:
        t0 += xi
        t.append(t0)

    plt.plot(t, y, color=colors[c_i], figure=fig, label=k)  # uninterpolated

    for xi, yi_min, yi_max in zip(t, y_min, y_max):
        plt.plot([xi, xi], [yi_max, yi_min], color=colors[c_i], linestyle='--')

    plt.legend()


def iteration_plot():
    ranking_path = '/home/orkan/Dropbox/sherlock/Relative k multiple/DUC06/iterations.csv'

    ranking_data = pd.read_csv(ranking_path)
    topics = list(ranking_data.columns.values)[1:-2]

    for topic in topics:
        df = ranking_data[['t', 'i', 'k', topic]]
        fig = plt.figure(2, figsize=(10, 6), dpi=80)
        for c_i, k in enumerate(df.k.unique()[:-1]):
        # for c_i, k in enumerate(df.k.unique()):
            # for i
            # x = [i + 1 for i in df.i.unique().tolist()]

            groupby = df.loc[df['k'] == k][['i', topic]].groupby(['i'])
            t = df.loc[df['k'] == k][['i', 't']].groupby('i').mean()['t'].tolist()
            y = groupby.mean()[topic].tolist()
            y_min = groupby.min()[topic].tolist()
            y_max = groupby.max()[topic].tolist()
            # plot 1 curve
            plot_iteration_curve(t, y, y_min, y_max, k, topic, fig, c_i)
        # fig.savefig('/home/orkan/Dropbox/sherlock/Relative k multiple/Iteration plots/' + topic + '.png')
        fig.savefig('/home/orkan/Dropbox/sherlock/Relative k multiple/Iteration error plots/' + topic + '.png')
        plt.clf()


def execute():
    base_system_path = '/home/orkan/Dropbox/sherlock/accept_reject multiple/all.csv'
    base_system_it_path = ''

    ranking_path = '/home/orkan/Dropbox/sherlock/Relative k multiple/DUC06/all.csv'
    ranking_it_path = '/home/orkan/Dropbox/sherlock/Relative k multiple/DUC06/iterations.csv'

    thr = 1
    sweep_path = '/home/orkan/Dropbox/sherlock/Redundancy sweep/thr {}/all.csv'.format(thr)
    sweep_it_path = '/home/orkan/Dropbox/sherlock/Redundancy sweep/thr {}/iterations.csv'.format(thr)

    # heartbeat_plot(base_system_path, ranking_path) # needs all.csv
    # iteration_plot() # needs iterations.csv
    avg_r2(ranking_path) # needs all.csv
    # mse_values() # needs all.csv

    # get_average_num_sents() # needs iterations.csv


execute()
