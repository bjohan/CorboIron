#!/bin/bash
OUTDIR=to_osh_park
BN=CorboIron
echo $OUTDIR $BN
cp CorboIron-B.Cu.gbl $OUTDIR/$BN.GBL
cp CorboIron-B.Mask.gbs $OUTDIR/$BN.GBS
cp CorboIron-B.SilkS.gbo $OUTDIR/$BN.GBO
cp CorboIron-Edge.Cuts.gm1 $OUTDIR/$BN.GKO
cp CorboIron-F.Cu.gtl $OUTDIR/$BN.GTL
cp CorboIron-F.Mask.gts $OUTDIR/$BN.GTS
cp CorboIron-F.SilkS.gto $OUTDIR/$BN.GTO
cp CorboIron.drl $OUTDIR/$BN.XLN
