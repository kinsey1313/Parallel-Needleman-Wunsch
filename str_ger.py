import numpy as np
import random


bases = ['A', 'T', 'G', 'C']
SIZE = 1000000
EDGE_BUFF = 0.12*SIZE
MAX_LEN = int(0.05*SIZE)


def make_gene_file(filename, size):
    f = open(filename, 'w+')
    for i in range(0,size):
        gene = np.random.choice(bases)
        f.write(gene)
    f.close()

def get_random_seq(min_len=5, max_len=100):
    leng = random.randint(min_len, max_len)
    seq = [random.choice(bases) for _ in range(leng)]
    seq = "".join(seq)
    #print(seq)
    return seq

def insertion(seq,pos):
    #print("ins")
    seq = seq[0:pos] + get_random_seq() + seq[pos:]
    #print(seq)
    return seq

def deletion(seq, pos, min_len = 10, max_len = MAX_LEN):
    #print("del")
    
    leng = random.randint(min_len, max_len)
    seq = seq[0:pos - leng] + seq[pos:]
    return seq

def duplication(seq,pos, min_size=10, max_len=MAX_LEN, n = 3):
    #print("dup")
    n = random.randint(1,n)
    size = random.randint(min_size, max_len)
    seq = seq[0:pos] + seq[pos:pos+max_len]*n + seq[pos+max_len:]
    return seq

def reversal(seq, pos, min_size = 10, max_len=MAX_LEN):
    #print("rev")
    leng = random.randint(min_size, max_len)
    seq = seq[0:pos] + seq[pos:pos+leng][::-1] + seq[pos+leng:]
    return seq

fns = [insertion, duplication, deletion, deletion, reversal]

def in_del_dup_rev(s, indel_prob):
    i = 0
    r = indel_prob
    mut_prob = 0.1
    while i < len(s):
        if(i > EDGE_BUFF and i < len(s) - EDGE_BUFF):
            if(r > random.random()):
                fn = np.random.choice(fns)
                if(fn==insertion):
                    i+=MAX_LEN
                s = fn(s, i)
        i+=1
    return s

def mutate_gene_file(in_file, out_file, mut_prob, big_mutes=False,i_p = 0.01):
    out_f = open(out_file, 'w+')
    out_s = ''
    with open(in_file, 'r') as file:
        data = list(file.read().replace('\n', ''))
        for i in range(0,len(data)):
            rand = random.random()
            if(rand < mut_prob):
                data[i] = np.random.choice(bases)
            out_s+=data[i]

    if(big_mutes):
        out_s = in_del_dup_rev(out_s, indel_prob=i_p)
    
    for char in out_s:
        #print("here")
        out_f.write(char)

SIZE = 100
fn = f"data/g_{SIZE}.txt"
out_fn = f"data/m_single_{SIZE}.txt"

#make_gene_file(fn,SIZE)
mutate_gene_file(fn,out_fn, 0.05)