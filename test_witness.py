import itertools
import json
from pathlib import Path
import random
import subprocess
import unittest
BIN=Path(__file__).parent/'build/witness'
def run(n,edges,changes,s=0,t=None):
    if t is None:t=n-1
    text=f'{n} {len(edges)} {len(changes)} {s} {t}\n'+'\n'.join(' '.join(map(str,e)) for e in edges+changes)+'\n'
    p=subprocess.run([str(BIN)],input=text,text=True,capture_output=True,timeout=5)
    if p.returncode:raise ValueError(p.stderr)
    return json.loads(p.stdout)
def oracle(n,edges,changes,mask):
    updated=[list(e) for e in edges]
    for i,(edge,on) in enumerate(changes):
        if mask&(1<<i):updated[edge][2]=on
    seen={0}
    while True:
        more=seen|{b for a,b,on in updated if on and a in seen}
        if more==seen:return n-1 in seen
        seen=more
class Tests(unittest.TestCase):
    def fixture(self):return 4,[(0,1,1),(1,3,1),(0,2,0),(2,3,0)],[(0,0),(1,0),(2,1),(3,1)]
    def test_demo_real_migration(self):
        result=run(*self.fixture());self.assertTrue(result['baseline_safe']);self.assertTrue(result['final_safe']);self.assertIsNotNone(result['safe_order']);self.assertIn(1,result['unsafe_masks'])
    def test_order_certificate(self):
        n,e,c=self.fixture();out=run(n,e,c);mask=0
        for i in out['safe_order']:mask|=1<<i;self.assertTrue(oracle(n,e,c,mask))
        self.assertEqual(mask,(1<<len(c))-1)
    def test_minimum_counterexample(self):
        out=run(*self.fixture());self.assertEqual(len(out['counterexample']['minimum_cardinality_changes']),1);self.assertNotIn(3,out['counterexample']['reachable_cut'])
    def test_noop_negative(self):
        out=run(2,[(0,1,1)],[(0,1)]);self.assertEqual(out['unsafe_masks'],[]);self.assertIsNone(out['counterexample'])
    def test_unrelated_edge_negative(self):
        out=run(4,[(0,3,1),(1,2,1)],[(1,0)]);self.assertEqual(out['unsafe_masks'],[])
    def test_impossible(self):self.assertIsNone(run(2,[(0,1,1)],[(0,0)])['safe_order'])
    def test_baseline_unsafe(self):
        out=run(2,[(0,1,0)],[(0,1)]);self.assertIsNone(out['safe_order']);self.assertEqual(out['counterexample']['minimum_cardinality_changes'],[])
    def test_empty_change_set(self):self.assertEqual(run(2,[(0,1,1)],[])['safe_order'],[])
    def test_random_exhaustive_oracle(self):
        rng=random.Random(20261031)
        for case in range(200):
            n=rng.randint(2,5);pairs=[(i,j) for i in range(n) for j in range(n) if i!=j];rng.shuffle(pairs)
            edges=[(i,j,rng.randrange(2)) for i,j in pairs[:rng.randint(1,min(8,len(pairs)))]]
            ids=rng.sample(range(len(edges)),rng.randint(0,min(5,len(edges))));changes=[(i,rng.randrange(2)) for i in ids]
            result=run(n,edges,changes);bad=[mask for mask in range(1<<len(changes)) if not oracle(n,edges,changes,mask)]
            self.assertEqual(result['unsafe_masks'],bad)
            expected=False
            for order in itertools.permutations(range(len(changes))):
                mask=0;valid=oracle(n,edges,changes,0)
                for i in order:mask|=1<<i;valid=valid and oracle(n,edges,changes,mask)
                expected|=valid
            self.assertEqual(result['safe_order'] is not None,expected)
            if bad:self.assertEqual(len(result['counterexample']['minimum_cardinality_changes']),min(m.bit_count() for m in bad))
    def test_invalid_grammar(self):
        for text in ['9 0 0 0 1','2 1 1 0 1\n0 1 2\n0 0','2 1 2 0 1\n0 1 1\n0 0\n0 1','2 0 0 0 1 extra','2 0 0 0 0','2 1 0 0 1\n0 999 1']:
            p=subprocess.run([str(BIN)],input=text,text=True,capture_output=True);self.assertEqual(p.returncode,2)
if __name__=='__main__':unittest.main()
