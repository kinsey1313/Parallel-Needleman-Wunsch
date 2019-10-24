import numpy as np
import random


bases = ['A', 'T', 'G', 'C']

def make_gene_file(filename, size):
    f = open(filename, 'w+')
    for i in range(0,size):
        gene = np.random.choice(bases)
        f.write(gene)
    f.close()

def mutate_gene_file(in_file, out_file, mut_prob):

    out_f = open(out_file, 'w+')

    with open(in_file, 'r') as file:
        data = list(file.read().replace('\n', ''))
        

        for i in range(0,len(data)):
            rand = random.random()
            #print(rand)
            if(rand < mut_prob):
                data[i] = np.random.choice(bases)
            out_f.write(data[i])

make_gene_file("out_genes.txt",10000)
mutate_gene_file("out_genes.txt","out_mutes.txt", 0.1)

            
        
    