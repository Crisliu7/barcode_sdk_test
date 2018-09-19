import csv
import numpy as np
import matplotlib.pyplot as plt

# catogary_name = 'speed'
# catogary_name = 'skew'
catogary_name = 'barcode_integrity'
# catogary_name = 'distance'
# catogary_name = 'resolution'


if catogary_name == 'skew':
    unit_name = '(deg)'
    x_axis = np.array([0, 20, 40], dtype=int)   
elif catogary_name == 'distance':
    unit_name = '(in)'
    x_axis = np.array([4, 6, 8, 10], dtype=int)
elif catogary_name == 'speed':
    unit_name = '(ips)'
    x_axis = np.array(['4', '10', '20', '40'], dtype=int)
elif catogary_name == 'barcode_integrity':
    unit_name = ''
    x_axis = np.array(['full', 'half', 'quarter'])
elif catogary_name == 'format':
    unit_name = ''
    x_axis = np.array(['gray', 'rgb8'])
elif catogary_name == 'resolution':
    unit_name = '(px)'
    x_axis = np.array(['480', '720', '1080'])

# plot
with open('multi_sdk_data.csv', 'r+', newline='') as csvfile:
    reader = csv.reader(csvfile)
    fig1 = plt.figure()
    # decode percentage - subplot1
    ax1 = plt.subplot(2,1,1)
    ax1.yaxis.grid()
    # ax1.set_xticks(np.arange(0, 12, step=2))
    ax1.set_yticks(np.arange(0, 1.1, step=0.1)) 
    ax1.set_ylim(0, 1.1)
#     ax1.set_xlabel('Distance (in)')
    ax1.set_xlabel(catogary_name+' '+unit_name)
    # ax1.set_xlabel('Skew (deg)')
    ax1.set_ylabel('% Decoded')
    ax1.set_title(catogary_name+' by Recognition Rate')


    # decode time - subplot2
    ax2 = plt.subplot(2,1,2)
    ax2.yaxis.grid()
    # ax2.set_xticks(np.arange(0, 12, step=2))
    ax2.set_yticks(np.arange(0, 0.6, step=0.1)) 
    ax2.set_ylim(0, 0.6)
#     ax2.set_xlabel('Distance (in)')
    ax2.set_xlabel(catogary_name+' ('+unit_name+')')
    # ax2.set_xlabel('Skew (deg)')
    ax2.set_ylabel('Decode Time (ms)')
    ax2.set_title(catogary_name+' by Decode Time')    
    
    index = 0
    legend = ['zbar', 'scandit', 'dynamsoft']
    
    for row in reader:
        row = [float(i) for i in row]
        if(index % 2 == 0):
            ax1.plot(x_axis, row, 'o-', label=legend[index//2])
        else:
            ax2.plot(x_axis, row, 'o-', label=legend[index//2])
        index += 1

ax1.legend(loc='center left', bbox_to_anchor=(1, 0.5))
ax2.legend(loc='center left', bbox_to_anchor=(1, 0.5))
plt.tight_layout()
plt.show()
        

        
        