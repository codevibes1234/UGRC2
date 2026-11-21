
rocedemand
==========

## Blind against demand-aware RoCE dheu, cell by cell

### ROCE_OCS 5d (sweep_ocs) (`rd_sweep_ocs.csv`, 8 cells)

| routing | rdelay | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- |
| updown | 1 | 182.22 | 73.99 | 0.41 | - | - |
| updown | 10 | 177.17 | 79.16 | 0.45 | 142.80 (sipring) | 1.80 |
| updown | 100 | 195.08 | 91.94 | 0.47 | - | - |
| updown | 1000 | 186.65 | 82.41 | 0.44 | - | - |
| shortest | 1 | 3.44 (EXPECTED_STALLED) | 3.44 (EXPECTED_STALLED) | 1.00 | - | - |
| shortest | 10 | 3.44 (EXPECTED_STALLED) | 3.44 (EXPECTED_STALLED) | 1.00 | - | - |
| shortest | 100 | 3.42 (EXPECTED_STALLED) | 3.40 (EXPECTED_STALLED) | 0.99 | - | - |
| shortest | 1000 | 11.44 (EXPECTED_STALLED) | 11.39 (EXPECTED_STALLED) | 1.00 | - | - |

### ROCE_OCS 5f (generalise_ocs) (`rd_generalise_ocs.csv`, 28 cells)

| workload | nodes | routing | rdelay | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| dlrm | 16 | updown | 1 | 182.22 | 73.99 | 0.41 | - | - |
| dlrm | 16 | updown | 10 | 177.17 | 79.16 | 0.45 | 142.80 (sipring) | 1.80 |
| dlrm | 16 | updown | 1000 | 186.65 | 82.41 | 0.44 | - | - |
| dlrm | 16 | shortest | 10 | 3.44 (EXPECTED_STALLED) | 3.44 (EXPECTED_STALLED) | 1.00 | - | - |
| vgg | 16 | updown | 1 | 527.15 | 150.94 | 0.29 | - | - |
| vgg | 16 | updown | 10 | 448.77 | 151.82 | 0.34 | 467.70 (sipring) | 3.08 |
| vgg | 16 | updown | 1000 | 627.50 | 163.51 | 0.26 | - | - |
| vgg | 16 | shortest | 10 | 107.97 (EXPECTED_STALLED) | 151.72 | 1.41 | - | - |
| transformer | 16 | updown | 1 | 149.45 | 63.34 | 0.42 | - | - |
| transformer | 16 | updown | 10 | 144.69 | 63.55 | 0.44 | 101.07 (sipring) | 1.59 |
| transformer | 16 | updown | 1000 | 137.44 | 71.50 | 0.52 | - | - |
| transformer | 16 | shortest | 10 | 132.23 | 62.20 (EXPECTED_STALLED) | 0.47 | - | - |
| candle | 16 | updown | 1 | 1389.31 | 959.37 | 0.69 | - | - |
| candle | 16 | updown | 10 | 1051.08 | 690.41 | 0.66 | 1620.35 (sipring) | 2.35 |
| candle | 16 | updown | 1000 | 1302.81 | 1144.01 | 0.88 | - | - |
| candle | 16 | shortest | 10 | 6.02 (EXPECTED_STALLED) | 125.93 (EXPECTED_STALLED) | 20.93 | - | - |
| dlrm | 32 | updown | 1 | 408.28 | 175.21 | 0.43 | - | - |
| dlrm | 32 | updown | 10 | 400.71 | 157.66 | 0.39 | - | - |
| dlrm | 32 | updown | 1000 | 440.60 | 188.25 | 0.43 | - | - |
| dlrm | 32 | shortest | 10 | 6.00 (EXPECTED_STALLED) | 6.00 (EXPECTED_STALLED) | 1.00 | - | - |
| transformer | 32 | updown | 1 | 545.24 | 125.25 | 0.23 | - | - |
| transformer | 32 | updown | 10 | 523.89 | 125.58 | 0.24 | - | - |
| transformer | 32 | updown | 1000 | 552.09 | 168.26 | 0.30 | - | - |
| transformer | 32 | shortest | 10 | 124.01 (EXPECTED_STALLED) | 125.51 | 1.01 | - | - |
| vgg | 32 | updown | 1 | 909.60 | 157.49 | 0.17 | - | - |
| vgg | 32 | updown | 10 | 716.37 | 158.33 | 0.22 | - | - |
| vgg | 32 | updown | 1000 | 1140.63 | 169.56 | 0.15 | - | - |
| vgg | 32 | shortest | 10 | 103.39 (EXPECTED_STALLED) | 158.33 | 1.53 | - | - |

### LOGICAL_OCS (`rd_logical_ocs.csv`, 7 cells)

| workload | nodes | degree | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- |
| dlrm | 16 | 4 | 177.17 | 79.16 | 0.45 | 110.62 (Dfly) | 1.40 |
| vgg | 16 | 4 | 448.77 | 151.82 | 0.34 | 379.38 (Dfly) | 2.50 |
| transformer | 16 | 4 | 144.69 | 63.55 | 0.44 | 90.84 (Dfly) | 1.43 |
| candle | 16 | 4 | 1051.08 | 690.41 | 0.66 | 845.99 (Dfly) | 1.23 |
| dlrm | 32 | 5 | 308.94 | 146.83 | 0.48 | 212.82 (Dfly) | 1.45 |
| vgg | 32 | 5 | 565.72 | 154.92 | 0.27 | 574.02 (Dfly) | 3.71 |
| transformer | 32 | 5 | 408.44 | 125.46 | 0.31 | 254.47 (Dfly) | 2.03 |

### OCS_MT_MDP (`rd_ocs_mt_mdp.csv`, 7 cells)

| series | placement | jobs | jobnodes | dims | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | 9.57 | 3.80 | 0.40 | 5.42 (FB) | 1.42 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | 10.63 | 4.91 | 0.46 | 5.42 (FB) | 1.10 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | 11.37 | 5.40 | 0.48 | 5.42 (FB) | 1.00 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | 8.03 | 2.38 | 0.30 | 6.10 (FB) | 2.56 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | 9.37 | 2.99 | 0.32 | 6.10 (FB) | 2.04 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | 11.42 | 3.26 | 0.29 | 6.10 (Jelly) | 1.87 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | 11.55 | 9.06 | 0.78 | 5.42 (FB) | 0.60 |

### PFC_HEADROOM, taskgraphs (`rd_pfc_realism_dynflat.csv`, 56 cells)

| series | workload | nodes | deg | routing | variant | extra | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| A | dlrm | 16 | 4 | updown | base |  | 177.17 | 79.16 | 0.45 | 110.62 (Dfly) | 1.40 |
| A | vgg | 16 | 4 | updown | base |  | 448.77 | 151.82 | 0.34 | 379.38 (Dfly) | 2.50 |
| A | transformer | 16 | 4 | updown | base |  | 144.69 | 63.55 | 0.44 | 90.84 (Dfly) | 1.43 |
| A | candle | 16 | 4 | updown | base |  | 1051.08 | 690.41 | 0.66 | 845.99 (Dfly) | 1.23 |
| A | dlrm | 32 | 5 | updown | base |  | 308.94 | 146.83 | 0.48 | 212.82 (Dfly) | 1.45 |
| A | vgg | 32 | 5 | updown | base |  | 565.72 | 154.92 | 0.27 | 574.02 (Dfly) | 3.71 |
| A | transformer | 32 | 5 | updown | base |  | 408.44 | 125.46 | 0.31 | 254.47 (Dfly) | 2.03 |
| A | dlrm | 16 | 4 | updown | delay | -pfcdelayns 1000 | 177.26 | 73.37 | 0.41 | 111.32 (Dfly) | 1.52 |
| A | vgg | 16 | 4 | updown | delay | -pfcdelayns 1000 | 456.87 | 151.82 | 0.33 | 379.58 (Dfly) | 2.50 |
| A | transformer | 16 | 4 | updown | delay | -pfcdelayns 1000 | 145.08 | 63.36 | 0.44 | 90.84 (Dfly) | 1.43 |
| A | candle | 16 | 4 | updown | delay | -pfcdelayns 1000 | 1055.48 | 674.32 | 0.64 | 838.28 (Dfly) | 1.24 |
| A | dlrm | 32 | 5 | updown | delay | -pfcdelayns 1000 | 309.43 | 150.37 | 0.49 | 212.52 (Dfly) | 1.41 |
| A | vgg | 32 | 5 | updown | delay | -pfcdelayns 1000 | 568.97 | 154.94 | 0.27 | 574.04 (Dfly) | 3.70 |
| A | transformer | 32 | 5 | updown | delay | -pfcdelayns 1000 | 408.71 | 125.57 | 0.31 | 254.47 (Dfly) | 2.03 |
| A | dlrm | 16 | 4 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 180.40 | 77.15 | 0.43 | 113.39 (Dfly) | 1.47 |
| A | vgg | 16 | 4 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 479.60 | 151.82 | 0.32 | 379.49 (Dfly) | 2.50 |
| A | transformer | 16 | 4 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 146.73 | 64.08 | 0.44 | 90.86 (Dfly) | 1.42 |
| A | candle | 16 | 4 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 1116.58 | 758.87 | 0.68 | 877.63 (Dfly) | 1.16 |
| A | dlrm | 32 | 5 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 323.35 | 148.76 | 0.46 | 220.04 (Dfly) | 1.48 |
| A | vgg | 32 | 5 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 616.37 | 156.78 | 0.25 | 574.03 (Dfly) | 3.66 |
| A | transformer | 32 | 5 | updown | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 411.61 | 125.53 | 0.30 | 254.47 (Dfly) | 2.03 |
| A | dlrm | 16 | 4 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 169.31 | 72.89 | 0.43 | 107.74 (Dfly) | 1.48 |
| A | vgg | 16 | 4 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 431.04 | 151.82 | 0.35 | 379.13 (Dfly) | 2.50 |
| A | transformer | 16 | 4 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 142.76 | 63.60 | 0.45 | 90.84 (Dfly) | 1.43 |
| A | candle | 16 | 4 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 916.51 | 579.64 | 0.63 | 820.66 (Dfly) | 1.42 |
| A | dlrm | 32 | 5 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 291.67 | 144.14 | 0.49 | 201.25 (Dfly) | 1.40 |
| A | vgg | 32 | 5 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 530.15 | 154.37 | 0.29 | 574.04 (Dfly) | 3.72 |
| A | transformer | 32 | 5 | updown | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 401.96 | 125.53 | 0.31 | 254.47 (Dfly) | 2.03 |
| B | dlrm | 16 | 4 | shortest | base |  | 3.44 (STALLED) | 3.44 (STALLED) | 1.00 | - | - |
| B | vgg | 16 | 4 | shortest | base |  | 107.97 (STALLED) | 151.72 | 1.41 | - | - |
| B | transformer | 16 | 4 | shortest | base |  | 132.23 | 62.20 (STALLED) | 0.47 | - | - |
| B | candle | 16 | 4 | shortest | base |  | 6.02 (STALLED) | 125.93 (STALLED) | 20.93 | - | - |
| B | dlrm | 32 | 5 | shortest | base |  | 6.00 (STALLED) | 6.00 (STALLED) | 1.00 | - | - |
| B | vgg | 32 | 5 | shortest | base |  | 103.39 (STALLED) | 155.61 | 1.51 | - | - |
| B | transformer | 32 | 5 | shortest | base |  | 124.01 (STALLED) | 125.52 | 1.01 | - | - |
| B | dlrm | 16 | 4 | shortest | delay | -pfcdelayns 1000 | 3.44 (STALLED) | 3.44 (STALLED) | 1.00 | - | - |
| B | vgg | 16 | 4 | shortest | delay | -pfcdelayns 1000 | 109.00 (STALLED) | 151.72 | 1.39 | - | - |
| B | transformer | 16 | 4 | shortest | delay | -pfcdelayns 1000 | 132.32 | 64.60 | 0.49 | - | - |
| B | candle | 16 | 4 | shortest | delay | -pfcdelayns 1000 | 6.02 (STALLED) | 95.06 (STALLED) | 15.80 | - | - |
| B | dlrm | 32 | 5 | shortest | delay | -pfcdelayns 1000 | 6.00 (STALLED) | 6.00 (STALLED) | 1.00 | - | - |
| B | vgg | 32 | 5 | shortest | delay | -pfcdelayns 1000 | 103.39 (STALLED) | 156.05 | 1.51 | - | - |
| B | transformer | 32 | 5 | shortest | delay | -pfcdelayns 1000 | 124.01 (STALLED) | 125.44 | 1.01 | - | - |
| B | dlrm | 16 | 4 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 3.44 (STALLED) | 3.44 (STALLED) | 1.00 | - | - |
| B | vgg | 16 | 4 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 111.83 (STALLED) | 151.72 | 1.36 | - | - |
| B | transformer | 16 | 4 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 134.55 | 63.26 | 0.47 | - | - |
| B | candle | 16 | 4 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 6.02 (STALLED) | 138.88 (STALLED) | 23.08 | - | - |
| B | dlrm | 32 | 5 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 6.00 (STALLED) | 6.00 (STALLED) | 1.00 | - | - |
| B | vgg | 32 | 5 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 103.39 (STALLED) | 103.39 (STALLED) | 1.00 | - | - |
| B | transformer | 32 | 5 | shortest | xoff8 | -pfcdelayns 1000 -pfcxoff 8 -pfcxon 5 | 124.01 (STALLED) | 124.01 (STALLED) | 1.00 | - | - |
| B | dlrm | 16 | 4 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 3.44 (STALLED) | 3.44 (STALLED) | 1.00 | - | - |
| B | vgg | 16 | 4 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 383.28 | 151.82 | 0.40 | - | - |
| B | transformer | 16 | 4 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 128.30 | 64.37 | 0.50 | - | - |
| B | candle | 16 | 4 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 6.02 (STALLED) | 113.66 (STALLED) | 18.89 | - | - |
| B | dlrm | 32 | 5 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 6.00 (STALLED) | 6.00 (STALLED) | 1.00 | - | - |
| B | vgg | 32 | 5 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 103.39 (STALLED) | 152.75 | 1.48 | - | - |
| B | transformer | 32 | 5 | shortest | xoff32 | -pfcdelayns 1000 -pfcxoff 32 -pfcxon 29 | 124.01 (STALLED) | 125.42 | 1.01 | - | - |

### PFC_HEADROOM, MDP (`rd_pfc_realism_mdp.csv`, 28 cells)

| series | placement | jobs | jobnodes | decomp | pfc_variant | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | base | 9.57 | 3.80 | 0.40 | 5.42 (FB) | 1.42 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | base | 10.63 | 4.91 | 0.46 | 5.42 (FB) | 1.10 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | base | 11.37 | 5.40 | 0.48 | 5.42 (FB) | 1.00 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | base | 8.03 | 2.38 | 0.30 | 6.10 (FB) | 2.56 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | base | 9.37 | 2.99 | 0.32 | 6.10 (FB) | 2.04 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | base | 11.42 | 3.26 | 0.29 | 6.10 (Jelly) | 1.87 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | base | 11.55 | 9.06 | 0.78 | 5.42 (FB) | 0.60 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | delay | 9.57 | 3.81 | 0.40 | 5.42 (FB) | 1.42 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | delay | 10.63 | 4.91 | 0.46 | 5.42 (FB) | 1.10 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | delay | 11.38 | 5.77 | 0.51 | 5.42 (FB) | 0.94 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | delay | 8.03 | 2.38 | 0.30 | 6.10 (FB) | 2.56 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | delay | 9.37 | 3.01 | 0.32 | 6.10 (FB) | 2.03 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | delay | 11.39 | 3.25 | 0.28 | 6.10 (Jelly) | 1.88 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | delay | 11.60 | 8.02 | 0.69 | 5.42 (FB) | 0.68 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 9.54 | 3.52 | 0.37 | 5.42 (FB) | 1.54 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 10.68 | 5.21 | 0.49 | 5.42 (FB) | 1.04 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 12.11 | 5.77 | 0.48 | 5.42 (FB) | 0.94 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 8.09 | 2.38 | 0.29 | 6.10 (FB) | 2.56 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 9.34 | 2.88 | 0.31 | 6.10 (FB) | 2.12 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 11.69 | 5.38 | 0.46 | 6.10 (Jelly) | 1.13 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | xoff8 | 12.20 | 11.77 | 0.97 | 5.42 (FB) | 0.46 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 9.59 | 3.80 | 0.40 | 5.42 (FB) | 1.43 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 10.60 | 4.66 | 0.44 | 5.42 (FB) | 1.16 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 11.38 | 4.79 | 0.42 | 5.42 (FB) | 1.13 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 8.03 | 2.38 | 0.30 | 6.10 (FB) | 2.56 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 9.39 | 2.56 | 0.27 | 6.10 (FB) | 2.39 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 11.36 | 3.01 | 0.26 | 6.10 (Jelly) | 2.03 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | xoff32 | 11.42 | 4.92 | 0.43 | 5.42 (FB) | 1.10 |

### RECONF_DELAY (`rd_reconf_delay.csv`, 70 cells)

| series | workload | nodes | deg | variant | extra | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| D | dlrm | 16 | 4 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 156.36 | 69.45 | 0.44 | - | - |
| D | dlrm | 16 | 4 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 157.18 | 69.46 | 0.44 | - | - |
| D | dlrm | 16 | 4 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 157.54 | 71.42 | 0.45 | - | - |
| D | dlrm | 16 | 4 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 161.24 | 72.63 | 0.45 | - | - |
| D | dlrm | 16 | 4 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 177.07 | 74.44 | 0.42 | - | - |
| D | dlrm | 16 | 4 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 300.87 | 132.98 | 0.44 | - | - |
| D | dlrm | 16 | 4 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 1508.97 | 661.78 | 0.44 | - | - |
| D | vgg | 16 | 4 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 376.71 | 132.56 | 0.35 | - | - |
| D | vgg | 16 | 4 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 375.96 | 132.58 | 0.35 | - | - |
| D | vgg | 16 | 4 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 377.30 | 132.75 | 0.35 | - | - |
| D | vgg | 16 | 4 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 384.46 | 134.46 | 0.35 | - | - |
| D | vgg | 16 | 4 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 453.42 | 151.73 | 0.33 | - | - |
| D | vgg | 16 | 4 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 1098.35 | 329.48 | 0.30 | - | - |
| D | vgg | 16 | 4 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 7532.08 | 2219.39 | 0.29 | - | - |
| D | transformer | 16 | 4 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 132.22 | 63.16 | 0.48 | - | - |
| D | transformer | 16 | 4 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 132.19 | 63.16 | 0.48 | - | - |
| D | transformer | 16 | 4 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 132.29 | 63.34 | 0.48 | - | - |
| D | transformer | 16 | 4 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 133.49 | 63.79 | 0.48 | - | - |
| D | transformer | 16 | 4 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 144.92 | 63.66 | 0.44 | - | - |
| D | transformer | 16 | 4 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 211.19 | 75.86 | 0.36 | - | - |
| D | transformer | 16 | 4 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 764.71 | 309.30 | 0.40 | - | - |
| D | candle | 16 | 4 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 941.04 | 616.52 | 0.66 | - | - |
| D | candle | 16 | 4 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 942.34 | 617.22 | 0.65 | - | - |
| D | candle | 16 | 4 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 948.65 | 616.89 | 0.65 | - | - |
| D | candle | 16 | 4 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 954.78 | 604.72 | 0.63 | - | - |
| D | candle | 16 | 4 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 1043.43 | 701.10 | 0.67 | - | - |
| D | candle | 16 | 4 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 2074.32 | 1333.18 | 0.64 | - | - |
| D | candle | 16 | 4 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 12387.73 | 7059.38 | 0.57 | - | - |
| D | dlrm | 32 | 5 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 285.69 | 134.15 | 0.47 | - | - |
| D | dlrm | 32 | 5 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 286.68 | 134.11 | 0.47 | - | - |
| D | dlrm | 32 | 5 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 285.87 | 138.69 | 0.49 | - | - |
| D | dlrm | 32 | 5 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 288.43 | 142.25 | 0.49 | - | - |
| D | dlrm | 32 | 5 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 308.42 | 147.34 | 0.48 | - | - |
| D | dlrm | 32 | 5 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 487.07 | 241.77 | 0.50 | - | - |
| D | dlrm | 32 | 5 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 2249.97 | 1135.58 | 0.50 | - | - |
| D | vgg | 32 | 5 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 475.63 | 138.03 | 0.29 | - | - |
| D | vgg | 32 | 5 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 475.57 | 137.90 | 0.29 | - | - |
| D | vgg | 32 | 5 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 476.96 | 134.74 | 0.28 | - | - |
| D | vgg | 32 | 5 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 484.24 | 138.84 | 0.29 | - | - |
| D | vgg | 32 | 5 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 566.40 | 155.66 | 0.27 | - | - |
| D | vgg | 32 | 5 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 1357.03 | 335.51 | 0.25 | - | - |
| D | vgg | 32 | 5 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 9172.61 | 2232.67 | 0.24 | - | - |
| D | transformer | 32 | 5 | d10_p400000 | -rdelayns 10 -rperiodns 400000 | 396.19 | 125.22 | 0.32 | - | - |
| D | transformer | 32 | 5 | d100_p400000 | -rdelayns 100 -rperiodns 400000 | 396.20 | 125.22 | 0.32 | - | - |
| D | transformer | 32 | 5 | d1000_p400000 | -rdelayns 1000 -rperiodns 400000 | 396.16 | 125.06 | 0.32 | - | - |
| D | transformer | 32 | 5 | d10000_p400000 | -rdelayns 10000 -rperiodns 400000 | 399.09 | 125.07 | 0.31 | - | - |
| D | transformer | 32 | 5 | d100000_p400000 | -rdelayns 100000 -rperiodns 400000 | 408.03 | 125.45 | 0.31 | - | - |
| D | transformer | 32 | 5 | d1000000_p400000 | -rdelayns 1000000 -rperiodns 400000 | 502.11 | 146.17 | 0.29 | - | - |
| D | transformer | 32 | 5 | d10000000_p400000 | -rdelayns 10000000 -rperiodns 400000 | 1094.15 | 488.07 | 0.45 | - | - |
| P | dlrm | 16 | 4 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 175.85 | 67.42 | 0.38 | - | - |
| P | dlrm | 16 | 4 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 151.40 | 67.16 | 0.44 | - | - |
| P | dlrm | 16 | 4 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 152.11 | 61.67 | 0.41 | - | - |
| P | vgg | 16 | 4 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 422.06 | 133.60 | 0.32 | - | - |
| P | vgg | 16 | 4 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 388.28 | 132.28 | 0.34 | - | - |
| P | vgg | 16 | 4 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 443.75 | 132.20 | 0.30 | - | - |
| P | transformer | 16 | 4 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 143.53 | 63.17 | 0.44 | - | - |
| P | transformer | 16 | 4 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 128.75 | 66.31 | 0.52 | - | - |
| P | transformer | 16 | 4 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 123.94 | 65.95 | 0.53 | - | - |
| P | candle | 16 | 4 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 1225.65 | 825.50 | 0.67 | - | - |
| P | candle | 16 | 4 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 806.66 | 475.08 | 0.59 | - | - |
| P | candle | 16 | 4 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 882.87 | 604.99 | 0.69 | - | - |
| P | dlrm | 32 | 5 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 310.37 | 132.64 | 0.43 | - | - |
| P | dlrm | 32 | 5 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 285.43 | 133.21 | 0.47 | - | - |
| P | dlrm | 32 | 5 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 298.66 | 117.41 | 0.39 | - | - |
| P | vgg | 32 | 5 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 548.32 | 128.22 | 0.23 | - | - |
| P | vgg | 32 | 5 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 508.30 | 188.47 | 0.37 | - | - |
| P | vgg | 32 | 5 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 625.75 | 433.46 | 0.69 | - | - |
| P | transformer | 32 | 5 | d10_p100000 | -rdelayns 10 -rperiodns 100000 | 428.69 | 125.05 | 0.29 | - | - |
| P | transformer | 32 | 5 | d10_p1600000 | -rdelayns 10 -rperiodns 1600000 | 404.14 | 125.05 | 0.31 | - | - |
| P | transformer | 32 | 5 | d10_p6400000 | -rdelayns 10 -rperiodns 6400000 | 374.68 | 125.05 | 0.33 | - | - |

### DECOMPOSITION (`rd_decomp.csv`, 45 cells)

| series | nodes | deg | jobnodes | decomp | allreduce | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F16d4 | 16 | 4 | 16 | tp2.dp2.pp2.ep2 | rhd | 11.55 | 9.06 | 0.78 | 5.42 (FB) | 0.60 |
| F16d4 | 16 | 4 | 16 | tp2.dp2.pp2.ep2 | ring | 11.55 | 9.06 | 0.78 | 5.42 (FB) | 0.60 |
| F16d4 | 16 | 4 | 16 | tp4.dp2.pp2.ep1 | rhd | 11.39 | 5.94 | 0.52 | 5.42 (FB) | 0.91 |
| F16d4 | 16 | 4 | 16 | tp4.dp2.pp2.ep1 | ring | 11.99 | 5.95 | 0.50 | 5.42 (FB) | 0.91 |
| F16d4 | 16 | 4 | 16 | tp4.dp4.pp1.ep1 | rhd | 19.67 | 8.86 | 0.45 | 8.13 (FB) | 0.92 |
| F16d4 | 16 | 4 | 16 | tp4.dp4.pp1.ep1 | ring | 19.56 | 10.08 | 0.52 | 16.22 (FB) | 1.61 |
| F16d4 | 16 | 4 | 16 | tp8.dp2.pp1.ep1 | rhd | 12.04 | 5.95 | 0.49 | 5.42 (FB) | 0.91 |
| F16d4 | 16 | 4 | 16 | tp8.dp2.pp1.ep1 | ring | 12.10 | 5.93 | 0.49 | 5.42 (FB) | 0.91 |
| F16d4 | 16 | 4 | 16 | tp16.dp1.pp1.ep1 | rhd | 3.03 | 2.71 | 0.90 | 1.28 (FB) | 0.47 |
| F16d4 | 16 | 4 | 16 | tp16.dp1.pp1.ep1 | ring | 3.63 | 1.43 | 0.39 | 2.64 (Dfly) | 1.85 |
| F16d6 | 16 | 6 | 16 | tp2.dp2.pp2.ep2 | rhd | 9.35 | 2.78 | 0.30 | 5.42 (FB) | 1.95 |
| F16d6 | 16 | 6 | 16 | tp2.dp2.pp2.ep2 | ring | 9.35 | 2.78 | 0.30 | 5.42 (FB) | 1.95 |
| F16d6 | 16 | 6 | 16 | tp4.dp2.pp2.ep1 | rhd | 9.10 | 2.45 | 0.27 | 5.42 (FB) | 2.21 |
| F16d6 | 16 | 6 | 16 | tp4.dp2.pp2.ep1 | ring | 9.23 | 4.12 | 0.45 | 5.42 (FB) | 1.32 |
| F16d6 | 16 | 6 | 16 | tp4.dp4.pp1.ep1 | rhd | 14.04 | 4.39 | 0.31 | 8.13 (FB) | 1.85 |
| F16d6 | 16 | 6 | 16 | tp4.dp4.pp1.ep1 | ring | 14.04 | 7.05 | 0.50 | 8.09 (FB) | 1.15 |
| F16d6 | 16 | 6 | 16 | tp8.dp2.pp1.ep1 | rhd | 8.91 | 3.54 | 0.40 | 5.42 (FB) | 1.53 |
| F16d6 | 16 | 6 | 16 | tp8.dp2.pp1.ep1 | ring | 9.48 | 3.42 | 0.36 | 5.42 (FB) | 1.59 |
| F16d6 | 16 | 6 | 16 | tp16.dp1.pp1.ep1 | rhd | 2.63 | 2.60 | 0.99 | 1.28 (FB) | 0.49 |
| F16d6 | 16 | 6 | 16 | tp16.dp1.pp1.ep1 | ring | 3.36 | 0.75 | 0.22 | 1.40 (Jelly) | 1.88 |
| F27d6 | 27 | 6 | 27 | tp3.dp3.pp3.ep1 | ring | 15.62 | 7.15 | 0.46 | 7.19 (FB) | 1.01 |
| F27d6 | 27 | 6 | 27 | tp9.dp3.pp1.ep1 | ring | 15.73 | 7.15 | 0.45 | 7.19 (FB) | 1.01 |
| F27d6 | 27 | 6 | 27 | tp3.dp9.pp1.ep1 | ring | 22.70 | 9.83 | 0.43 | 19.20 (FB) | 1.95 |
| F27d6 | 27 | 6 | 27 | tp27.dp1.pp1.ep1 | ring | 4.32 | 1.24 | 0.29 | 1.63 (Dfly) | 1.31 |
| F27d6 | 27 | 6 | 27 | tp3.dp3.pp1.ep3 | ring | 16.17 | 7.41 | 0.46 | 7.19 (FB) | 0.97 |
| J12d4 | 16 | 4 | 12 | tp3.dp2.pp2.ep1 | ring | 12.35 | 4.51 | 0.37 | 7.24 (Jelly) | 1.60 |
| J12d4 | 16 | 4 | 12 | tp3.dp2.pp2.ep1 | auto | 12.35 | 4.51 | 0.37 | 7.24 (Jelly) | 1.60 |
| J12d4 | 16 | 4 | 12 | tp2.dp3.pp2.ep1 | ring | 15.70 | 7.83 | 0.50 | 7.23 (FB) | 0.92 |
| J12d4 | 16 | 4 | 12 | tp2.dp3.pp2.ep1 | auto | 15.70 | 7.83 | 0.50 | 7.23 (FB) | 0.92 |
| J12d4 | 16 | 4 | 12 | tp3.dp4.pp1.ep1 | ring | 17.87 | 8.36 | 0.47 | 17.23 (Dfly) | 2.06 |
| J12d4 | 16 | 4 | 12 | tp3.dp4.pp1.ep1 | auto | 18.35 | 6.34 | 0.35 | 14.61 (Jelly) | 2.31 |
| J12d4 | 16 | 4 | 12 | tp4.dp3.pp1.ep1 | ring | 16.78 | 7.38 | 0.44 | 7.23 (FB) | 0.98 |
| J12d4 | 16 | 4 | 12 | tp4.dp3.pp1.ep1 | auto | 16.06 | 7.72 | 0.48 | 7.23 (FB) | 0.94 |
| J12d4 | 16 | 4 | 12 | tp12.dp1.pp1.ep1 | ring | 3.45 | 2.45 | 0.71 | 2.55 (Dfly) | 1.04 |
| J12d4 | 16 | 4 | 12 | tp12.dp1.pp1.ep1 | auto | 3.45 | 2.45 | 0.71 | 2.55 (Dfly) | 1.04 |
| J12d6 | 16 | 6 | 12 | tp3.dp2.pp2.ep1 | ring | 10.18 | 2.63 | 0.26 | 6.32 (Dfly) | 2.41 |
| J12d6 | 16 | 6 | 12 | tp3.dp2.pp2.ep1 | auto | 10.18 | 2.63 | 0.26 | 6.32 (Dfly) | 2.41 |
| J12d6 | 16 | 6 | 12 | tp2.dp3.pp2.ep1 | ring | 11.99 | 5.16 | 0.43 | 7.19 (FB) | 1.39 |
| J12d6 | 16 | 6 | 12 | tp2.dp3.pp2.ep1 | auto | 11.99 | 5.16 | 0.43 | 7.19 (FB) | 1.39 |
| J12d6 | 16 | 6 | 12 | tp3.dp4.pp1.ep1 | ring | 13.46 | 5.82 | 0.43 | 9.05 (Dfly) | 1.55 |
| J12d6 | 16 | 6 | 12 | tp3.dp4.pp1.ep1 | auto | 14.19 | 3.91 | 0.28 | 9.94 (Dfly) | 2.54 |
| J12d6 | 16 | 6 | 12 | tp4.dp3.pp1.ep1 | ring | 12.02 | 5.10 | 0.42 | 7.19 (FB) | 1.41 |
| J12d6 | 16 | 6 | 12 | tp4.dp3.pp1.ep1 | auto | 11.86 | 4.69 | 0.40 | 7.19 (FB) | 1.53 |
| J12d6 | 16 | 6 | 12 | tp12.dp1.pp1.ep1 | ring | 2.49 | 1.24 | 0.50 | 1.30 (Dfly) | 1.04 |
| J12d6 | 16 | 6 | 12 | tp12.dp1.pp1.ep1 | auto | 2.49 | 1.24 | 0.50 | 1.30 (Dfly) | 1.04 |

### PHASED_MDP (`rd_phased.csv`, 31 cells)

| series | nodes | deg | placement | jobs | jobnodes | decomp | variant | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | conc | 11.55 | 9.06 | 0.78 | 5.42 (FB) | 0.60 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph4c0 | 16.54 | 9.42 | 0.57 | 7.23 (FB) | 0.77 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph8c0 | 16.41 | 9.86 | 0.60 | 7.04 (FB) | 0.71 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph8c100 | 18.39 | 11.31 | 0.61 | 9.74 (FB) | 0.86 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | conc | 24.81 | 4.27 | 0.17 | 8.13 (FB) | 1.90 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ph8c0 | 31.74 | 16.76 | 0.53 | 10.70 (FB) | 0.64 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ph8c100 | 34.77 | 18.25 | 0.52 | 14.00 (FB) | 0.77 |
| D | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | conc | 18.69 | 3.39 | 0.18 | 8.13 (FB) | 2.39 |
| D | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | conc | 21.90 | 2.91 | 0.13 | 8.13 (FB) | 2.79 |
| D | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | conc | 25.57 | 3.34 | 0.13 | 8.13 (FB) | 2.44 |
| D | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | conc | 15.45 | 2.25 | 0.15 | 15.79 (Dfly) | 7.02 |
| D | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | conc | 17.98 | 2.41 | 0.13 | 19.08 (Jelly) | 7.90 |
| D | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | conc | 22.11 | 3.64 | 0.16 | 27.88 (Jelly) | 7.66 |
| D | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 24.05 | 6.56 | 0.27 | 9.75 (FB) | 1.49 |
| D | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 27.63 | 6.04 | 0.22 | 9.75 (FB) | 1.61 |
| D | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 30.35 | 9.26 | 0.30 | 9.75 (FB) | 1.05 |
| D | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 19.11 | 6.56 | 0.34 | 18.07 (Jelly) | 2.75 |
| D | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 21.36 | 7.12 | 0.33 | 20.30 (Jelly) | 2.85 |
| D | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 27.38 | 9.82 | 0.36 | 29.39 (Jelly) | 2.99 |
| E | 16 | 4 | compact | 1 | 4 | tp2.dp2.pp1.ep1 | conc | 9.57 | 3.80 | 0.40 | 5.42 (FB) | 1.42 |
| E | 16 | 4 | compact | 2 | 4 | tp2.dp2.pp1.ep1 | conc | 10.63 | 4.91 | 0.46 | 5.42 (FB) | 1.10 |
| E | 16 | 4 | compact | 4 | 4 | tp2.dp2.pp1.ep1 | conc | 11.37 | 5.40 | 0.48 | 5.42 (FB) | 1.00 |
| E | 16 | 4 | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | conc | 8.03 | 2.38 | 0.30 | 6.10 (FB) | 2.56 |
| E | 16 | 4 | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | conc | 9.37 | 2.99 | 0.32 | 6.10 (FB) | 2.04 |
| E | 16 | 4 | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | conc | 11.42 | 3.26 | 0.29 | 6.10 (Jelly) | 1.87 |
| E | 16 | 4 | compact | 1 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 10.06 | 3.78 | 0.38 | 6.11 (FB) | 1.62 |
| E | 16 | 4 | compact | 2 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 11.24 | 5.11 | 0.45 | 6.11 (FB) | 1.20 |
| E | 16 | 4 | compact | 4 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 12.24 | 4.97 | 0.41 | 6.11 (FB) | 1.23 |
| E | 16 | 4 | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 8.94 | 2.83 | 0.32 | 6.17 (Jelly) | 2.18 |
| E | 16 | 4 | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 10.27 | 3.39 | 0.33 | 6.17 (Jelly) | 1.82 |
| E | 16 | 4 | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 13.26 | 3.04 | 0.23 | 6.17 (Jelly) | 2.03 |

### SEEDS, taskgraphs (`rd_seeds_dynflat.csv`, 16 cells)

| workload | variant | extra | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- |
| dlrm | seed2 | -seed 2 -logicalseed 2 | 162.18 | 73.94 | 0.46 | 110.62 (Dfly) | 1.50 |
| vgg | seed2 | -seed 2 -logicalseed 2 | 444.19 | 151.82 | 0.34 | 379.38 (Dfly) | 2.50 |
| transformer | seed2 | -seed 2 -logicalseed 2 | 137.04 | 64.44 | 0.47 | 90.84 (Dfly) | 1.41 |
| candle | seed2 | -seed 2 -logicalseed 2 | 1052.04 | 649.61 | 0.62 | 844.18 (Jelly) | 1.30 |
| dlrm | seed3 | -seed 3 -logicalseed 3 | 168.06 | 79.82 | 0.47 | 110.62 (Dfly) | 1.39 |
| vgg | seed3 | -seed 3 -logicalseed 3 | 443.76 | 151.72 | 0.34 | 379.38 (Dfly) | 2.50 |
| transformer | seed3 | -seed 3 -logicalseed 3 | 139.66 | 63.26 | 0.45 | 90.84 (Dfly) | 1.44 |
| candle | seed3 | -seed 3 -logicalseed 3 | 1055.14 | 662.63 | 0.63 | 845.99 (Dfly) | 1.28 |
| dlrm | seed4 | -seed 4 -logicalseed 4 | 179.56 | 74.86 | 0.42 | 110.62 (Dfly) | 1.48 |
| vgg | seed4 | -seed 4 -logicalseed 4 | 451.80 | 151.82 | 0.34 | 379.38 (Dfly) | 2.50 |
| transformer | seed4 | -seed 4 -logicalseed 4 | 152.91 | 66.14 | 0.43 | 90.84 (Dfly) | 1.37 |
| candle | seed4 | -seed 4 -logicalseed 4 | 1062.63 | 704.57 | 0.66 | 845.99 (Dfly) | 1.20 |
| dlrm | seed5 | -seed 5 -logicalseed 5 | 174.06 | 75.62 | 0.43 | 110.62 (Dfly) | 1.46 |
| vgg | seed5 | -seed 5 -logicalseed 5 | 445.14 | 151.72 | 0.34 | 379.38 (Dfly) | 2.50 |
| transformer | seed5 | -seed 5 -logicalseed 5 | 141.13 | 65.96 | 0.47 | 90.84 (Dfly) | 1.38 |
| candle | seed5 | -seed 5 -logicalseed 5 | 1062.12 | 696.42 | 0.66 | 796.32 (Jelly) | 1.14 |

### SEEDS, MDP (`rd_seeds_mdp.csv`, 28 cells)

| series | placement | jobs | jobnodes | decomp | variant | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed2 | 8.32 | 3.28 | 0.39 | 5.42 (FB) | 1.65 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed2 | 9.57 | 4.65 | 0.49 | 5.42 (FB) | 1.16 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed2 | 11.29 | 5.26 | 0.47 | 5.42 (FB) | 1.03 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed2 | 7.50 | 2.66 | 0.35 | 5.42 (FB) | 2.04 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed2 | 8.30 | 2.84 | 0.34 | 5.42 (FB) | 1.91 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed2 | 9.37 | 3.49 | 0.37 | 11.21 (FB) | 3.21 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed2 | 14.14 | 8.45 | 0.60 | 5.42 (FB) | 0.64 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed3 | 8.79 | 3.29 | 0.37 | 5.42 (FB) | 1.64 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed3 | 10.12 | 4.58 | 0.45 | 5.42 (FB) | 1.18 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed3 | 11.80 | 5.14 | 0.44 | 5.42 (FB) | 1.05 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed3 | 9.16 | 2.77 | 0.30 | 6.10 (Jelly) | 2.20 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed3 | 10.44 | 2.48 | 0.24 | 6.10 (Jelly) | 2.45 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed3 | 11.83 | 4.30 | 0.36 | 6.77 (Jelly) | 1.58 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed3 | 12.70 | 8.12 | 0.64 | 5.42 (FB) | 0.67 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed4 | 8.97 | 3.82 | 0.43 | 5.42 (FB) | 1.42 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed4 | 10.05 | 4.89 | 0.49 | 5.42 (FB) | 1.11 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed4 | 11.32 | 5.34 | 0.47 | 5.42 (FB) | 1.01 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed4 | 8.51 | 2.35 | 0.28 | 6.10 (FB) | 2.60 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed4 | 10.39 | 2.85 | 0.27 | 6.10 (FB) | 2.14 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed4 | 11.81 | 4.23 | 0.36 | 6.10 (FB) | 1.44 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed4 | 13.94 | 7.82 | 0.56 | 5.42 (FB) | 0.69 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed5 | 10.21 | 3.80 | 0.37 | 5.42 (FB) | 1.43 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed5 | 11.18 | 4.70 | 0.42 | 5.42 (FB) | 1.15 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed5 | 13.46 | 5.67 | 0.42 | 5.42 (FB) | 0.96 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed5 | 8.04 | 2.35 | 0.29 | 5.42 (FB) | 2.31 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed5 | 8.71 | 2.59 | 0.30 | 5.42 (FB) | 2.09 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed5 | 9.85 | 3.94 | 0.40 | 11.35 (FB) | 2.88 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed5 | 14.50 | 8.60 | 0.59 | 5.42 (FB) | 0.63 |

### OCS_SCALE (`rd_ocs_scale.csv`, 10 cells)

| series | nodes | deg | placement | jobs | jobnodes | decomp | allreduce | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| fullmdp | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | rhd | 24.81 | 4.27 | 0.17 | 8.13 (FB) | 1.90 |
| fullmdp | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ring | 23.52 | 6.28 | 0.27 | 16.22 (FB) | 2.58 |
| fullmdp | 128 | 12 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | rhd | 19.35 | 4.65 | 0.24 | 8.13 (FB) | 1.75 |
| tenancy | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | rhd | 18.69 | 3.39 | 0.18 | 8.13 (FB) | 2.39 |
| tenancy | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | rhd | 21.90 | 2.91 | 0.13 | 8.13 (FB) | 2.79 |
| tenancy | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | rhd | 25.57 | 3.34 | 0.13 | 8.13 (FB) | 2.44 |
| tenancy | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | rhd | 15.45 | 2.25 | 0.15 | 15.79 (Dfly) | 7.02 |
| tenancy | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | rhd | 17.98 | 2.41 | 0.13 | 19.08 (Jelly) | 7.90 |
| tenancy | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | rhd | 22.11 | 3.64 | 0.16 | 27.88 (Jelly) | 7.66 |
| fullmdp | 256 | 12 | compact | 1 | 256 | tp4.dp4.pp4.ep4 | rhd | 26.15 | 5.52 | 0.21 | 8.13 (FB) | 1.47 |

## Summary: does reconfiguring beat the best static family under RoCE?

| experiment | cells with a static comparison | blind dheu wins | demand-aware dheu wins | best static / demand-aware dheu (min-max, mean) |
| --- | --- | --- | --- | --- |
| ROCE_OCS 5d (sweep_ocs) | 1 | 0 | 1 | 1.80-1.80, 1.80 |
| ROCE_OCS 5f (generalise_ocs) | 4 | 2 | 4 | 1.59-3.08, 2.21 |
| LOGICAL_OCS | 7 | 1 | 7 | 1.23-3.71, 1.96 |
| OCS_MT_MDP | 7 | 0 | 6 | 0.60-2.56, 1.51 |
| PFC_HEADROOM, taskgraphs | 28 | 3 | 28 | 1.16-3.72, 1.97 |
| PFC_HEADROOM, MDP | 28 | 0 | 23 | 0.46-2.56, 1.53 |
| DECOMPOSITION | 45 | 0 | 31 | 0.47-2.54, 1.38 |
| PHASED_MDP | 31 | 4 | 25 | 0.60-7.90, 2.23 |
| SEEDS, taskgraphs | 16 | 0 | 16 | 1.14-2.50, 1.65 |
| SEEDS, MDP | 28 | 2 | 23 | 0.63-3.21, 1.58 |
| OCS_SCALE | 10 | 3 | 10 | 1.47-7.90, 3.79 |

## Against the corrected DCTCP model

LOGICAL_OCS workloads, ms. DCTCP = exact queue, 1 ms timer scan (PFC_HEADROOM.md section 6).

| workload | RoCE dheu (demand) | DCTCP dheu | RoCE / DCTCP | RoCE best static | DCTCP best static |
| --- | --- | --- | --- | --- | --- |
| dlrm 16 | 79.16 | 192.97 | 0.41 | 110.62 (Dfly) | 178.47 (Dfly) |
| vgg 16 | 151.82 | 151.15 | 1.00 | 379.38 (Dfly) | 393.85 (Dfly) |
| transformer 16 | 63.55 | 63.37 | 1.00 | 90.84 (Dfly) | 113.89 (Dfly) |
| candle 16 | 690.41 | 468.10 | 1.47 | 845.99 (Dfly) | 637.81 (Dfly) |
| dlrm 32 | 146.83 | 568.03 | 0.26 | 212.82 (Dfly) | 697.69 (Mfly) |
| vgg 32 | 154.92 | 283.58 | 0.55 | 574.02 (Dfly) | 777.01 (Mfly) |
| transformer 32 | 125.46 | 125.29 | 1.00 | 254.47 (Dfly) | 340.37 (Dfly) |

MDP cells with a corrected-DCTCP dheu run (DECOMPOSITION, OCS_SCALE), job 0 ms:

| experiment | cell | RoCE dheu (demand) | DCTCP dheu | DCTCP / RoCE | RoCE best static |
| --- | --- | --- | --- | --- | --- |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp2.dp2.pp2.ep2 rhd | 9.06 | 24.11 | 2.66 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp2.dp2.pp2.ep2 ring | 9.06 | 24.11 | 2.66 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp2.pp2.ep1 rhd | 5.94 | 33.19 | 5.59 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp2.pp2.ep1 ring | 5.95 | 24.30 | 4.08 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp4.pp1.ep1 rhd | 8.86 | 25.53 | 2.88 | 8.13 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp4.pp1.ep1 ring | 10.08 | 27.73 | 2.75 | 16.22 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp8.dp2.pp1.ep1 rhd | 5.95 | 55.18 | 9.28 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp8.dp2.pp1.ep1 ring | 5.93 | 44.72 | 7.54 | 5.42 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp16.dp1.pp1.ep1 rhd | 2.71 | 66.18 | 24.41 | 1.28 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp16.dp1.pp1.ep1 ring | 1.43 | 0.97 | 0.68 | 2.64 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp2.dp2.pp2.ep2 rhd | 2.78 | 12.78 | 4.60 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp2.dp2.pp2.ep2 ring | 2.78 | 12.78 | 4.60 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp2.pp2.ep1 rhd | 2.45 | 22.04 | 9.00 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp2.pp2.ep1 ring | 4.12 | 22.33 | 5.42 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp4.pp1.ep1 rhd | 4.39 | 24.70 | 5.63 | 8.13 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp4.pp1.ep1 ring | 7.05 | 25.90 | 3.67 | 8.09 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp8.dp2.pp1.ep1 rhd | 3.54 | 45.13 | 12.74 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp8.dp2.pp1.ep1 ring | 3.42 | 45.19 | 13.22 | 5.42 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp16.dp1.pp1.ep1 rhd | 2.60 | 66.09 | 25.41 | 1.28 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp16.dp1.pp1.ep1 ring | 0.75 | 0.69 | 0.92 | 1.40 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp3.pp3.ep1 ring | 7.15 | 25.38 | 3.55 | 7.19 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp9.dp3.pp1.ep1 ring | 7.15 | 46.09 | 6.44 | 7.19 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp9.pp1.ep1 ring | 9.83 | 58.95 | 6.00 | 19.20 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp27.dp1.pp1.ep1 ring | 1.24 | 1.65 | 1.33 | 1.63 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp3.pp1.ep3 ring | 7.41 | 34.26 | 4.62 | 7.19 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp2.pp2.ep1 ring | 4.51 | 23.95 | 5.31 | 7.24 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp2.pp2.ep1 auto | 4.51 | 23.95 | 5.31 | 7.24 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp2.dp3.pp2.ep1 ring | 7.83 | 25.87 | 3.31 | 7.23 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp2.dp3.pp2.ep1 auto | 7.83 | 25.87 | 3.31 | 7.23 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp4.pp1.ep1 ring | 8.36 | 27.52 | 3.29 | 17.23 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp4.pp1.ep1 auto | 6.34 | 27.41 | 4.33 | 14.61 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp4.dp3.pp1.ep1 ring | 7.38 | 22.76 | 3.08 | 7.23 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp4.dp3.pp1.ep1 auto | 7.72 | 33.21 | 4.30 | 7.23 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp12.dp1.pp1.ep1 ring | 2.45 | 1.62 | 0.66 | 2.55 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp12.dp1.pp1.ep1 auto | 2.45 | 1.62 | 0.66 | 2.55 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp2.pp2.ep1 ring | 2.63 | 22.61 | 8.60 | 6.32 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp2.pp2.ep1 auto | 2.63 | 22.61 | 8.60 | 6.32 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp2.dp3.pp2.ep1 ring | 5.16 | 22.23 | 4.31 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp2.dp3.pp2.ep1 auto | 5.16 | 22.23 | 4.31 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp4.pp1.ep1 ring | 5.82 | 25.92 | 4.45 | 9.05 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp4.pp1.ep1 auto | 3.91 | 13.97 | 3.58 | 9.94 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp4.dp3.pp1.ep1 ring | 5.10 | 23.01 | 4.51 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp4.dp3.pp1.ep1 auto | 4.69 | 25.48 | 5.44 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp12.dp1.pp1.ep1 ring | 1.24 | 0.93 | 0.75 | 1.30 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp12.dp1.pp1.ep1 auto | 1.24 | 0.93 | 0.75 | 1.30 |
| OCS_SCALE | fullmdp n128 d9 compact j1 tp4.dp4.pp4.ep2 rhd | 4.27 | 34.24 | 8.01 | 8.13 |
| OCS_SCALE | fullmdp n128 d9 compact j1 tp4.dp4.pp4.ep2 ring | 6.28 | 45.23 | 7.20 | 16.22 |
| OCS_SCALE | fullmdp n128 d12 compact j1 tp4.dp4.pp4.ep2 rhd | 4.65 | 35.42 | 7.62 | 8.13 |
| OCS_SCALE | tenancy n128 d9 compact j1 tp2.dp4.pp2.ep2 rhd | 3.39 | 33.33 | 9.82 | 8.13 |
| OCS_SCALE | tenancy n128 d9 compact j2 tp2.dp4.pp2.ep2 rhd | 2.91 | 24.55 | 8.43 | 8.13 |
| OCS_SCALE | tenancy n128 d9 compact j4 tp2.dp4.pp2.ep2 rhd | 3.34 | 34.62 | 10.38 | 8.13 |
| OCS_SCALE | tenancy n128 d9 scatter j1 tp2.dp4.pp2.ep2 rhd | 2.25 | 24.33 | 10.81 | 15.79 |
| OCS_SCALE | tenancy n128 d9 scatter j2 tp2.dp4.pp2.ep2 rhd | 2.41 | 24.52 | 10.16 | 19.08 |
| OCS_SCALE | tenancy n128 d9 scatter j4 tp2.dp4.pp2.ep2 rhd | 3.64 | 23.34 | 6.41 | 27.88 |

DCTCP / RoCE over 54 cells: 0.66-25.41, mean 6.06; RoCE faster in 48.

RECONF_DELAY: circuit-down time swept at a 400 us period (series D) and the period swept at 10 ns (series P), ms. Best static = the best of the four families as fixed circuits under RoCE (LOGICAL_OCS); DCTCP = exact queue, 1 ms timer scan.

| workload | variant | RoCE dheu blind | RoCE dheu demand | DCTCP dheu | RoCE best static | demand-aware dheu beats it? |
| --- | --- | --- | --- | --- | --- | --- |
| dlrm 16 | d10_p400000 | 156.36 | 69.45 | 339.14 | 110.62 | yes (1.59x) |
| dlrm 16 | d100_p400000 | 157.18 | 69.46 | 176.36 | 110.62 | yes (1.59x) |
| dlrm 16 | d1000_p400000 | 157.54 | 71.42 | 178.76 | 110.62 | yes (1.55x) |
| dlrm 16 | d10000_p400000 | 161.24 | 72.63 | 245.01 (DRAIN_STALLED) | 110.62 | yes (1.52x) |
| dlrm 16 | d100000_p400000 | 177.07 | 74.44 | 223.86 | 110.62 | yes (1.49x) |
| dlrm 16 | d1000000_p400000 | 300.87 | 132.98 | 176.66 | 110.62 | no (0.83x) |
| dlrm 16 | d10000000_p400000 | 1508.97 | 661.78 | 1106.12 | 110.62 | no (0.17x) |
| vgg 16 | d10_p400000 | 376.71 | 132.56 | 132.09 | 379.38 | yes (2.86x) |
| vgg 16 | d100_p400000 | 375.96 | 132.58 | 133.60 | 379.38 | yes (2.86x) |
| vgg 16 | d1000_p400000 | 377.30 | 132.75 | 133.60 | 379.38 | yes (2.86x) |
| vgg 16 | d10000_p400000 | 384.46 | 134.46 | 144.59 | 379.38 | yes (2.82x) |
| vgg 16 | d100000_p400000 | 453.42 | 151.73 | 150.96 | 379.38 | yes (2.50x) |
| vgg 16 | d1000000_p400000 | 1098.35 | 329.48 | 327.16 | 379.38 | yes (1.15x) |
| vgg 16 | d10000000_p400000 | 7532.08 | 2219.39 | 2229.41 | 379.38 | no (0.17x) |
| transformer 16 | d10_p400000 | 132.22 | 63.16 | 73.05 | 90.84 | yes (1.44x) |
| transformer 16 | d100_p400000 | 132.19 | 63.16 | 73.05 | 90.84 | yes (1.44x) |
| transformer 16 | d1000_p400000 | 132.29 | 63.34 | 68.74 | 90.84 | yes (1.43x) |
| transformer 16 | d10000_p400000 | 133.49 | 63.79 | 69.49 | 90.84 | yes (1.42x) |
| transformer 16 | d100000_p400000 | 144.92 | 63.66 | 107.07 | 90.84 | yes (1.43x) |
| transformer 16 | d1000000_p400000 | 211.19 | 75.86 | 107.00 | 90.84 | yes (1.20x) |
| transformer 16 | d10000000_p400000 | 764.71 | 309.30 | 635.30 | 90.84 | no (0.29x) |
| candle 16 | d10_p400000 | 941.04 | 616.52 | 442.32 | 845.99 | yes (1.37x) |
| candle 16 | d100_p400000 | 942.34 | 617.22 | 419.51 | 845.99 | yes (1.37x) |
| candle 16 | d1000_p400000 | 948.65 | 616.89 | 430.53 | 845.99 | yes (1.37x) |
| candle 16 | d10000_p400000 | 954.78 | 604.72 | 432.62 | 845.99 | yes (1.40x) |
| candle 16 | d100000_p400000 | 1043.43 | 701.10 | 499.21 | 845.99 | yes (1.21x) |
| candle 16 | d1000000_p400000 | 2074.32 | 1333.18 | 916.94 | 845.99 | no (0.63x) |
| candle 16 | d10000000_p400000 | 12387.73 | 7059.38 | 5696.46 | 845.99 | no (0.12x) |
| dlrm 32 | d10_p400000 | 285.69 | 134.15 | 336.86 | 212.82 | yes (1.59x) |
| dlrm 32 | d100_p400000 | 286.68 | 134.11 | 217.41 | 212.82 | yes (1.59x) |
| dlrm 32 | d1000_p400000 | 285.87 | 138.69 | 202.40 | 212.82 | yes (1.53x) |
| dlrm 32 | d10000_p400000 | 288.43 | 142.25 | 343.62 | 212.82 | yes (1.50x) |
| dlrm 32 | d100000_p400000 | 308.42 | 147.34 | 236.50 | 212.82 | yes (1.44x) |
| dlrm 32 | d1000000_p400000 | 487.07 | 241.77 | 421.06 | 212.82 | no (0.88x) |
| dlrm 32 | d10000000_p400000 | 2249.97 | 1135.58 | 3550.00 | 212.82 | no (0.19x) |
| vgg 32 | d10_p400000 | 475.63 | 138.03 | 352.62 | 574.02 | yes (4.16x) |
| vgg 32 | d100_p400000 | 475.57 | 137.90 | 372.03 | 574.02 | yes (4.16x) |
| vgg 32 | d1000_p400000 | 476.96 | 134.74 | 488.63 | 574.02 | yes (4.26x) |
| vgg 32 | d10000_p400000 | 484.24 | 138.84 | 208.86 | 574.02 | yes (4.13x) |
| vgg 32 | d100000_p400000 | 566.40 | 155.66 | 360.74 | 574.02 | yes (3.69x) |
| vgg 32 | d1000000_p400000 | 1357.03 | 335.51 | 634.40 | 574.02 | yes (1.71x) |
| vgg 32 | d10000000_p400000 | 9172.61 | 2232.67 | 2284.77 | 574.02 | no (0.26x) |
| transformer 32 | d10_p400000 | 396.19 | 125.22 | 125.14 | 254.47 | yes (2.03x) |
| transformer 32 | d100_p400000 | 396.20 | 125.22 | 125.07 | 254.47 | yes (2.03x) |
| transformer 32 | d1000_p400000 | 396.16 | 125.06 | 125.54 | 254.47 | yes (2.03x) |
| transformer 32 | d10000_p400000 | 399.09 | 125.07 | 125.09 | 254.47 | yes (2.03x) |
| transformer 32 | d100000_p400000 | 408.03 | 125.45 | 125.97 | 254.47 | yes (2.03x) |
| transformer 32 | d1000000_p400000 | 502.11 | 146.17 | 195.88 | 254.47 | yes (1.74x) |
| transformer 32 | d10000000_p400000 | 1094.15 | 488.07 | 1270.40 | 254.47 | no (0.52x) |
| dlrm 16 | d10_p100000 | 175.85 | 67.42 | - | 110.62 | yes (1.64x) |
| dlrm 16 | d10_p1600000 | 151.40 | 67.16 | - | 110.62 | yes (1.65x) |
| dlrm 16 | d10_p6400000 | 152.11 | 61.67 | - | 110.62 | yes (1.79x) |
| vgg 16 | d10_p100000 | 422.06 | 133.60 | - | 379.38 | yes (2.84x) |
| vgg 16 | d10_p1600000 | 388.28 | 132.28 | - | 379.38 | yes (2.87x) |
| vgg 16 | d10_p6400000 | 443.75 | 132.20 | - | 379.38 | yes (2.87x) |
| transformer 16 | d10_p100000 | 143.53 | 63.17 | - | 90.84 | yes (1.44x) |
| transformer 16 | d10_p1600000 | 128.75 | 66.31 | - | 90.84 | yes (1.37x) |
| transformer 16 | d10_p6400000 | 123.94 | 65.95 | - | 90.84 | yes (1.38x) |
| candle 16 | d10_p100000 | 1225.65 | 825.50 | - | 845.99 | yes (1.02x) |
| candle 16 | d10_p1600000 | 806.66 | 475.08 | - | 845.99 | yes (1.78x) |
| candle 16 | d10_p6400000 | 882.87 | 604.99 | - | 845.99 | yes (1.40x) |
| dlrm 32 | d10_p100000 | 310.37 | 132.64 | - | 212.82 | yes (1.60x) |
| dlrm 32 | d10_p1600000 | 285.43 | 133.21 | - | 212.82 | yes (1.60x) |
| dlrm 32 | d10_p6400000 | 298.66 | 117.41 | - | 212.82 | yes (1.81x) |
| vgg 32 | d10_p100000 | 548.32 | 128.22 | - | 574.02 | yes (4.48x) |
| vgg 32 | d10_p1600000 | 508.30 | 188.47 | - | 574.02 | yes (3.05x) |
| vgg 32 | d10_p6400000 | 625.75 | 433.46 | - | 574.02 | yes (1.32x) |
| transformer 32 | d10_p100000 | 428.69 | 125.05 | - | 254.47 | yes (2.03x) |
| transformer 32 | d10_p1600000 | 404.14 | 125.05 | - | 254.47 | yes (2.03x) |
| transformer 32 | d10_p6400000 | 374.68 | 125.05 | - | 254.47 | yes (2.03x) |

| variant | workloads where demand-aware RoCE dheu beats the best static family |
| --- | --- |
| d10_p400000 | 7/7 |
| d100_p400000 | 7/7 |
| d1000_p400000 | 7/7 |
| d10000_p400000 | 7/7 |
| d100000_p400000 | 7/7 |
| d1000000_p400000 | 4/7 |
| d10000000_p400000 | 0/7 |
| d10_p100000 | 7/7 |
| d10_p1600000 | 7/7 |
| d10_p6400000 | 7/7 |

decomp
======

### Fabrics as built

| nodes | degree | arm | construction | links | diameter |
| --- | --- | --- | --- | --- | --- |
| 16 | 4 | jellyfish | random regular graph; degree budget 4 | 31 | 3 |
| 16 | 4 | flatbutterfly | 2-ary 4-flat; degree 4 | 32 | 4 |
| 16 | 4 | dragonfly | 4 groups x 4 nodes; 1 global port(s) per node | 32 | 3 |
| 16 | 4 | megafly | 4 groups x [2 leaf + 2 spine] | 22 | 5 |
| 16 | 6 | jellyfish | random regular graph; degree budget 6 | 48 | 3 |
| 16 | 6 | flatbutterfly | 4-ary 2-flat; degree 6 | 48 | 2 |
| 16 | 6 | dragonfly | 4 groups x 4 nodes; 3 global port(s) per node | 48 | 3 |
| 16 | 6 | megafly | 4 groups x [2 leaf + 2 spine] | 22 | 5 |
| 27 | 6 | jellyfish | random regular graph; degree budget 6 | 80 | 3 |
| 27 | 6 | flatbutterfly | 3-ary 3-flat; degree 6 | 81 | 3 |
| 27 | 6 | dragonfly | 9 groups x 3 nodes; 4 global port(s) per node | 81 | 3 |
| 27 | 6 | megafly | 3 groups x [5 leaf + 4 spine] | 63 | 5 |

**F16d4, rhd AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 19.33 (8.59 / 19.33) | 5.42 (0.68 / 5.42) | 21.74 (1.61 / 21.74) | 34.50 (6.07 / 34.50) | 11.55 (3.22 / 11.55) | FB |
| 4,2,2,1 | 18.77 (7.61 / 18.77) | 5.42 (1.02 / 5.42) | 21.67 (2.38 / 21.67) | 33.95 (5.80 / 33.95) | 11.39 (4.21 / 11.39) | FB |
| 4,4,1,1 | 21.31 (5.58 / 21.31) | 8.13 (1.02 / 8.13) | 26.46 (2.43 / 26.46) | 38.44 (4.25 / 38.44) | 19.67 (4.28 / 19.67) | FB |
| 8,2,1,1 | 18.65 (8.35 / 18.65) | 5.42 (1.19 / 5.42) | 21.84 (2.81 / 21.84) | 33.18 (4.24 / 33.18) | 12.04 (5.30 / 12.04) | FB |
| 16,1,1,1 | 2.66 (2.66 / NA) | 1.28 (1.28 / NA) | 1.89 (1.89 / NA) | 2.89 (2.89 / NA) | 3.03 (3.03 / NA) | FB |

**F16d4, rhd AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 19.13 (4.29 / 19.13) | 5.37 (0.67 / 5.37) | 21.48 (1.27 / 21.48) | 33.56 (2.83 / 33.56) | 6.08 (4.94 / 6.08) | FB |
| 4,2,2,1 | 18.46 (4.16 / 18.46) | 5.37 (1.01 / 5.37) | 21.48 (1.62 / 21.48) | 33.56 (2.68 / 33.56) | 7.36 (6.15 / 7.36) | FB |
| 4,4,1,1 | 20.85 (3.39 / 20.85) | 8.06 (1.01 / 8.06) | 25.41 (1.78 / 25.41) | 37.68 (2.65 / 37.68) | 9.03 (5.17 / 9.03) | FB |
| 8,2,1,1 | 18.47 (4.18 / 18.47) | 5.37 (1.18 / 5.37) | 21.65 (2.47 / 21.65) | 32.89 (4.28 / 32.89) | 7.16 (5.73 / 7.16) | FB |
| 16,1,1,1 | 2.64 (2.64 / NA) | 1.27 (1.27 / NA) | 1.87 (1.87 / NA) | 2.88 (2.88 / NA) | 3.93 (3.93 / NA) | FB |

**F16d4, ring AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 19.33 (8.59 / 19.33) | 5.42 (0.68 / 5.42) | 21.74 (1.61 / 21.74) | 34.50 (6.07 / 34.50) | 11.55 (3.22 / 11.55) | FB |
| 4,2,2,1 | 18.62 (8.68 / 18.62) | 5.42 (2.04 / 5.42) | 21.67 (2.68 / 21.67) | 33.89 (6.52 / 33.89) | 11.99 (4.80 / 11.99) | FB |
| 4,4,1,1 | 25.36 (6.33 / 25.36) | 16.22 (2.04 / 16.22) | 24.46 (2.34 / 24.46) | 32.50 (5.07 / 32.50) | 19.56 (4.22 / 19.56) | FB |
| 8,2,1,1 | 18.62 (9.40 / 18.62) | 5.42 (3.57 / 5.42) | 21.68 (7.03 / 21.68) | 33.74 (8.87 / 33.74) | 12.10 (6.17 / 12.10) | FB |
| 16,1,1,1 | 2.69 (2.69 / NA) | 5.14 (5.14 / NA) | 2.64 (2.64 / NA) | 2.69 (2.69 / NA) | 3.63 (3.63 / NA) | Dfly |

**F16d4, ring AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 19.13 (4.29 / 19.13) | 5.37 (0.67 / 5.37) | 21.48 (1.27 / 21.48) | 33.56 (2.83 / 33.56) | 6.08 (4.94 / 6.08) | FB |
| 4,2,2,1 | 18.46 (4.57 / 18.46) | 5.37 (2.02 / 5.37) | 21.48 (1.69 / 21.48) | 33.56 (3.12 / 33.56) | 6.55 (5.01 / 6.55) | FB |
| 4,4,1,1 | 25.19 (4.27 / 25.19) | 16.13 (2.02 / 16.13) | 24.49 (2.05 / 24.49) | 32.30 (3.02 / 32.30) | 11.00 (5.17 / 11.00) | FB |
| 8,2,1,1 | 18.47 (5.50 / 18.47) | 5.37 (3.56 / 5.37) | 21.48 (4.34 / 21.48) | 33.40 (5.62 / 33.40) | 6.22 (5.56 / 6.22) | FB |
| 16,1,1,1 | 2.65 (2.65 / NA) | 5.11 (5.11 / NA) | 2.70 (2.70 / NA) | 2.65 (2.65 / NA) | 0.97 (0.97 / NA) | Jelly=Mfly |

**F16d6, rhd AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 17.62 (3.02 / 17.62) | 5.42 (0.68 / 5.42) | 10.96 (1.22 / 10.96) | 34.50 (6.07 / 34.50) | 9.35 (2.49 / 9.35) | FB |
| 4,2,2,1 | 17.27 (4.10 / 17.27) | 5.42 (1.02 / 5.42) | 10.98 (1.53 / 10.98) | 33.95 (5.80 / 33.95) | 9.10 (3.08 / 9.10) | FB |
| 4,4,1,1 | 16.96 (3.48 / 16.96) | 8.13 (1.02 / 8.13) | 13.55 (1.31 / 13.55) | 38.44 (4.25 / 38.44) | 14.04 (3.20 / 14.04) | FB |
| 8,2,1,1 | 17.28 (4.80 / 17.28) | 5.42 (1.19 / 5.42) | 10.94 (2.30 / 10.94) | 33.18 (4.24 / 33.18) | 8.91 (3.70 / 8.91) | FB |
| 16,1,1,1 | 1.63 (1.63 / NA) | 1.28 (1.28 / NA) | 1.54 (1.54 / NA) | 2.89 (2.89 / NA) | 2.63 (2.63 / NA) | FB |

**F16d6, rhd AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 17.45 (2.58 / 17.45) | 5.37 (0.67 / 5.37) | 10.94 (1.14 / 10.94) | 33.56 (2.83 / 33.56) | 2.89 (2.74 / 2.89) | FB |
| 4,2,2,1 | 17.12 (2.61 / 17.12) | 5.37 (1.01 / 5.37) | 11.06 (1.34 / 11.06) | 33.56 (2.68 / 33.56) | 3.11 (2.39 / 3.11) | FB |
| 4,4,1,1 | 16.80 (2.17 / 16.80) | 8.06 (1.01 / 8.06) | 13.43 (1.20 / 13.43) | 37.68 (2.65 / 37.68) | 5.62 (2.86 / 5.62) | FB |
| 8,2,1,1 | 17.12 (3.25 / 17.12) | 5.37 (1.18 / 5.37) | 10.93 (1.87 / 10.93) | 32.89 (4.28 / 32.89) | 3.18 (3.09 / 3.18) | FB |
| 16,1,1,1 | 1.62 (1.62 / NA) | 1.27 (1.27 / NA) | 1.53 (1.53 / NA) | 2.88 (2.88 / NA) | 3.23 (3.23 / NA) | FB |

**F16d6, ring AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 17.62 (3.02 / 17.62) | 5.42 (0.68 / 5.42) | 10.96 (1.22 / 10.96) | 34.50 (6.07 / 34.50) | 9.35 (2.49 / 9.35) | FB |
| 4,2,2,1 | 17.26 (3.79 / 17.26) | 5.42 (1.02 / 5.42) | 11.04 (1.86 / 11.04) | 33.89 (6.52 / 33.89) | 9.23 (3.60 / 9.23) | FB |
| 4,4,1,1 | 17.45 (5.42 / 17.45) | 8.09 (1.02 / 8.09) | 16.19 (1.51 / 16.19) | 32.50 (5.07 / 32.50) | 14.04 (3.41 / 14.04) | FB |
| 8,2,1,1 | 17.44 (4.90 / 17.44) | 5.42 (2.38 / 5.42) | 11.03 (2.59 / 11.03) | 33.74 (8.87 / 33.74) | 9.48 (4.85 / 9.48) | FB |
| 16,1,1,1 | 1.40 (1.40 / NA) | 2.57 (2.57 / NA) | 2.57 (2.57 / NA) | 2.69 (2.69 / NA) | 3.36 (3.36 / NA) | Jelly |

**F16d6, ring AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 2,2,2,2 | 17.45 (2.58 / 17.45) | 5.37 (0.67 / 5.37) | 10.94 (1.14 / 10.94) | 33.56 (2.83 / 33.56) | 2.89 (2.74 / 2.89) | FB |
| 4,2,2,1 | 17.12 (2.60 / 17.12) | 5.37 (1.01 / 5.37) | 11.18 (1.58 / 11.18) | 33.56 (3.12 / 33.56) | 3.22 (3.22 / 2.81) | FB |
| 4,4,1,1 | 17.13 (2.58 / 17.13) | 8.06 (1.01 / 8.06) | 16.12 (1.49 / 16.12) | 32.30 (3.02 / 32.30) | 6.87 (3.36 / 6.87) | FB |
| 8,2,1,1 | 17.29 (3.35 / 17.29) | 5.37 (2.38 / 5.37) | 11.32 (2.19 / 11.32) | 33.40 (5.62 / 33.40) | 3.64 (3.64 / 2.56) | FB |
| 16,1,1,1 | 1.40 (1.40 / NA) | 2.58 (2.58 / NA) | 2.58 (2.58 / NA) | 2.65 (2.65 / NA) | 0.69 (0.69 / NA) | Jelly |

**F27d6, ring AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,3,3,1 | 17.95 (7.75 / 17.95) | 7.19 (0.90 / 7.19) | 14.45 (1.25 / 14.45) | 66.71 (14.59 / 66.71) | 15.62 (3.80 / 15.62) | FB |
| 9,3,1,1 | 16.81 (6.39 / 16.81) | 7.19 (2.42 / 7.19) | 15.64 (4.93 / 15.64) | 66.30 (23.83 / 66.30) | 15.73 (6.15 / 15.73) | FB |
| 3,9,1,1 | 31.60 (5.81 / 31.60) | 19.20 (0.90 / 19.20) | 20.82 (2.55 / 20.82) | 39.99 (3.86 / 39.99) | 22.70 (3.60 / 22.70) | FB |
| 27,1,1,1 | 2.81 (2.81 / NA) | 4.00 (4.00 / NA) | 1.63 (1.63 / NA) | 4.16 (4.16 / NA) | 4.32 (4.32 / NA) | Dfly |
| 3,3,1,3 | 18.13 (7.87 / 18.13) | 7.19 (0.90 / 7.19) | 15.12 (1.58 / 15.12) | 67.39 (13.87 / 67.39) | 16.17 (4.79 / 16.17) | FB |

**F27d6, ring AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,3,3,1 | 17.68 (4.39 / 17.68) | 7.16 (0.90 / 7.16) | 14.33 (1.24 / 14.33) | 64.50 (5.65 / 64.50) | 8.21 (5.89 / 8.21) | FB |
| 9,3,1,1 | 16.72 (4.01 / 16.72) | 7.16 (2.43 / 7.16) | 15.52 (3.50 / 15.52) | 64.45 (5.49 / 64.45) | 6.83 (6.83 / 6.24) | FB |
| 3,9,1,1 | 32.58 (4.31 / 32.58) | 19.14 (0.90 / 19.14) | 20.04 (1.92 / 20.04) | 39.52 (3.34 / 39.52) | 11.43 (2.55 / 11.43) | FB |
| 27,1,1,1 | 2.83 (2.83 / NA) | 3.98 (3.98 / NA) | 1.62 (1.62 / NA) | 4.11 (4.11 / NA) | 1.65 (1.65 / NA) | Dfly |
| 3,3,1,3 | 18.02 (4.73 / 18.02) | 7.16 (0.90 / 7.16) | 14.99 (1.57 / 14.99) | 65.12 (6.32 / 65.12) | 8.88 (7.00 / 8.88) | FB |

**J12d4, ring AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 7.24 (4.07 / 7.24) | 18.61 (7.05 / 18.61) | 12.28 (5.06 / 12.28) | 25.72 (7.43 / 25.72) | 12.35 (4.13 / 12.35) | Jelly |
| 2,3,2,1 | 16.73 (4.89 / 16.73) | 7.23 (0.68 / 7.23) | 21.89 (1.30 / 21.89) | 29.37 (3.78 / 29.37) | 15.70 (2.65 / 15.70) | FB |
| 3,4,1,1 | 25.77 (5.25 / 25.77) | 33.48 (7.89 / 33.48) | 17.23 (4.24 / 17.23) | 25.30 (4.72 / 25.30) | 17.87 (3.83 / 17.87) | Dfly |
| 4,3,1,1 | 17.07 (6.13 / 17.07) | 7.23 (2.04 / 7.23) | 22.09 (2.01 / 22.09) | 29.00 (5.26 / 29.00) | 16.78 (4.40 / 16.78) | FB |
| 12,1,1,1 | 2.59 (2.59 / NA) | 5.03 (5.03 / NA) | 2.55 (2.55 / NA) | 2.59 (2.59 / NA) | 3.45 (3.45 / NA) | Dfly |

**J12d4, ring AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 7.16 (2.68 / 7.16) | 17.90 (3.75 / 17.90) | 11.68 (2.52 / 11.68) | 23.14 (3.76 / 23.14) | 4.63 (3.58 / 4.63) | Jelly |
| 2,3,2,1 | 17.06 (2.79 / 17.06) | 7.17 (0.67 / 7.17) | 21.56 (1.18 / 21.56) | 28.80 (2.56 / 28.80) | 8.81 (3.47 / 8.81) | FB |
| 3,4,1,1 | 25.14 (3.29 / 25.14) | 33.47 (4.30 / 33.47) | 17.01 (2.70 / 17.01) | 25.14 (2.78 / 25.14) | 8.80 (3.50 / 8.80) | Dfly |
| 4,3,1,1 | 17.08 (3.67 / 17.08) | 7.17 (2.02 / 7.17) | 21.91 (2.00 / 21.91) | 28.72 (3.03 / 28.72) | 8.17 (4.11 / 8.17) | FB |
| 12,1,1,1 | 2.57 (2.57 / NA) | 5.03 (5.03 / NA) | 2.61 (2.61 / NA) | 2.57 (2.57 / NA) | 1.62 (1.62 / NA) | Jelly=Mfly |

**J12d4, auto AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 7.24 (4.07 / 7.24) | 18.61 (7.05 / 18.61) | 12.28 (5.06 / 12.28) | 25.72 (7.43 / 25.72) | 12.35 (4.13 / 12.35) | Jelly |
| 2,3,2,1 | 16.73 (4.89 / 16.73) | 7.23 (0.68 / 7.23) | 21.89 (1.30 / 21.89) | 29.37 (3.78 / 29.37) | 15.70 (2.65 / 15.70) | FB |
| 3,4,1,1 | 14.61 (4.64 / 14.61) | 29.43 (10.79 / 29.43) | 16.26 (2.85 / 16.26) | 27.91 (5.01 / 27.91) | 18.35 (3.78 / 18.35) | Jelly |
| 4,3,1,1 | 17.18 (5.18 / 17.18) | 7.23 (1.02 / 7.23) | 21.87 (1.93 / 21.87) | 29.31 (4.66 / 29.31) | 16.06 (4.22 / 16.06) | FB |
| 12,1,1,1 | 2.59 (2.59 / NA) | 5.03 (5.03 / NA) | 2.55 (2.55 / NA) | 2.59 (2.59 / NA) | 3.45 (3.45 / NA) | Dfly |

**J12d4, auto AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 7.16 (2.68 / 7.16) | 17.90 (3.75 / 17.90) | 11.68 (2.52 / 11.68) | 23.14 (3.76 / 23.14) | 4.63 (3.58 / 4.63) | Jelly |
| 2,3,2,1 | 17.06 (2.79 / 17.06) | 7.17 (0.67 / 7.17) | 21.56 (1.18 / 21.56) | 28.80 (2.56 / 28.80) | 8.81 (3.47 / 8.81) | FB |
| 3,4,1,1 | 14.33 (2.63 / 14.33) | 30.46 (4.83 / 30.46) | 17.30 (2.33 / 17.30) | 27.03 (3.11 / 27.03) | 7.17 (3.42 / 7.17) | Jelly |
| 4,3,1,1 | 17.24 (3.67 / 17.24) | 7.17 (1.01 / 7.17) | 21.49 (1.66 / 21.49) | 28.75 (2.65 / 28.75) | 9.18 (5.72 / 9.18) | FB |
| 12,1,1,1 | 2.57 (2.57 / NA) | 5.03 (5.03 / NA) | 2.61 (2.61 / NA) | 2.57 (2.57 / NA) | 1.62 (1.62 / NA) | Jelly=Mfly |

**J12d6, ring AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 11.73 (2.87 / 11.73) | 11.73 (2.79 / 11.73) | 6.32 (2.46 / 6.32) | 25.72 (7.43 / 25.72) | 10.18 (2.54 / 10.18) | Dfly |
| 2,3,2,1 | 15.13 (3.12 / 15.13) | 7.19 (0.68 / 7.19) | 14.50 (1.23 / 14.50) | 29.37 (3.78 / 29.37) | 11.99 (2.47 / 11.99) | FB |
| 3,4,1,1 | 17.12 (3.65 / 17.12) | 16.20 (1.84 / 16.20) | 9.05 (2.48 / 9.05) | 25.30 (4.72 / 25.30) | 13.46 (2.46 / 13.46) | Dfly |
| 4,3,1,1 | 15.47 (4.23 / 15.47) | 7.19 (1.02 / 7.19) | 14.38 (1.51 / 14.38) | 29.00 (5.26 / 29.00) | 12.02 (3.52 / 12.02) | FB |
| 12,1,1,1 | 1.34 (1.34 / NA) | 2.50 (2.50 / NA) | 1.30 (1.30 / NA) | 2.59 (2.59 / NA) | 2.49 (2.49 / NA) | Dfly |

**J12d6, ring AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 11.63 (1.85 / 11.63) | 11.64 (2.02 / 11.64) | 6.27 (1.61 / 6.27) | 23.14 (3.76 / 23.14) | 2.83 (2.51 / 2.83) | Dfly |
| 2,3,2,1 | 15.00 (1.87 / 15.00) | 7.16 (0.67 / 7.16) | 14.52 (1.15 / 14.52) | 28.80 (2.56 / 28.80) | 5.77 (2.09 / 5.77) | FB |
| 3,4,1,1 | 17.01 (1.98 / 17.01) | 16.13 (1.48 / 16.13) | 8.97 (1.58 / 8.97) | 25.14 (2.78 / 25.14) | 5.83 (2.81 / 5.83) | Dfly |
| 4,3,1,1 | 15.33 (2.69 / 15.33) | 7.16 (1.01 / 7.16) | 14.33 (1.49 / 14.33) | 28.72 (3.03 / 28.72) | 5.59 (3.84 / 5.59) | FB |
| 12,1,1,1 | 1.34 (1.34 / NA) | 2.52 (2.52 / NA) | 1.30 (1.30 / NA) | 2.57 (2.57 / NA) | 0.93 (0.93 / NA) | Dfly |

**J12d6, auto AllReduce, roce** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 11.73 (2.87 / 11.73) | 11.73 (2.79 / 11.73) | 6.32 (2.46 / 6.32) | 25.72 (7.43 / 25.72) | 10.18 (2.54 / 10.18) | Dfly |
| 2,3,2,1 | 15.13 (3.12 / 15.13) | 7.19 (0.68 / 7.19) | 14.50 (1.23 / 14.50) | 29.37 (3.78 / 29.37) | 11.99 (2.47 / 11.99) | FB |
| 3,4,1,1 | 14.45 (3.55 / 14.45) | 11.74 (2.46 / 11.74) | 9.94 (1.88 / 9.94) | 27.91 (5.01 / 27.91) | 14.19 (2.36 / 14.19) | Dfly |
| 4,3,1,1 | 15.13 (3.47 / 15.13) | 7.19 (1.02 / 7.19) | 14.50 (1.56 / 14.50) | 29.31 (4.66 / 29.31) | 11.86 (3.50 / 11.86) | FB |
| 12,1,1,1 | 1.34 (1.34 / NA) | 2.50 (2.50 / NA) | 1.30 (1.30 / NA) | 2.59 (2.59 / NA) | 2.49 (2.49 / NA) | Dfly |

**J12d6, auto AllReduce, dctcp** - makespan ms, and in brackets TP-class / DP-class ms

| TP,DP,PP,EP | Jelly | FB | Dfly | Mfly | dheu | best static |
| --- | --- | --- | --- | --- | --- | --- |
| 3,2,2,1 | 11.63 (1.85 / 11.63) | 11.64 (2.02 / 11.64) | 6.27 (1.61 / 6.27) | 23.14 (3.76 / 23.14) | 2.83 (2.51 / 2.83) | Dfly |
| 2,3,2,1 | 15.00 (1.87 / 15.00) | 7.16 (0.67 / 7.16) | 14.52 (1.15 / 14.52) | 28.80 (2.56 / 28.80) | 5.77 (2.09 / 5.77) | FB |
| 3,4,1,1 | 14.33 (2.04 / 14.33) | 11.64 (1.61 / 11.64) | 10.63 (1.44 / 10.63) | 27.03 (3.11 / 27.03) | 4.35 (1.66 / 4.35) | Dfly |
| 4,3,1,1 | 15.00 (2.35 / 15.00) | 7.16 (1.01 / 7.16) | 14.59 (1.35 / 14.59) | 28.75 (2.65 / 28.75) | 5.33 (2.98 / 5.33) | FB |
| 12,1,1,1 | 1.34 (1.34 / NA) | 2.52 (2.52 / NA) | 1.30 (1.30 / NA) | 2.57 (2.57 / NA) | 0.93 (0.93 / NA) | Dfly |

Non-OK runs: 0

### Robustness across decompositions: makespan / best static family in the same cell

(a win is being within 0.5% of the best; both transports pooled)

| cells | Jelly | FB | Dfly | Mfly |
| --- | --- | --- | --- | --- |
| 16 nodes, RHD (all groups power-of-two) | 0/20 wins, mean 2.81x, worst 3.57x | 20/20 wins, mean 1.00x, worst 1.00x | 0/20 wins, mean 2.57x, worst 4.03x | 0/20 wins, mean 5.13x, worst 6.37x |
| 16 nodes, ring | 3/20 wins, mean 2.58x, worst 3.57x | 16/20 wins, mean 1.18x, worst 1.94x | 1/20 wins, mean 2.45x, worst 4.01x | 1/20 wins, mean 4.65x, worst 6.37x |
| 27 nodes, ring (3-ary) | 0/10 wins, mean 2.15x, worst 2.52x | 8/10 wins, mean 1.29x, worst 2.46x | 2/10 wins, mean 1.67x, worst 2.17x | 0/10 wins, mean 6.42x, worst 9.37x |
| 12-node job on 16 nodes, ring | 3/20 wins, mean 1.72x, worst 2.38x | 8/20 wins, mean 1.60x, worst 2.57x | 9/20 wins, mean 1.68x, worst 3.05x | 1/20 wins, mean 3.07x, worst 4.08x |
| 12-node job on 16 nodes, auto | 5/20 wins, mean 1.62x, worst 2.40x | 8/20 wins, mean 1.55x, worst 2.57x | 7/20 wins, mean 1.69x, worst 3.03x | 1/20 wins, mean 3.10x, worst 4.08x |
| everything except the aligned case (row 1) | 11/70 wins, mean 2.00x, worst 3.57x | 40/70 wins, mean 1.42x, worst 2.57x | 19/70 wins, mean 1.90x, worst 4.01x | 3/70 wins, mean 4.01x, worst 9.37x |
| all cells | 11/90 wins, mean 2.18x, worst 3.57x | 60/90 wins, mean 1.33x, worst 2.57x | 19/90 wins, mean 2.05x, worst 4.03x | 3/90 wins, mean 4.26x, worst 9.37x |

scale
=====

### Fabrics as built

| nodes | degree | arm | construction | links | diameter | mean hops |
| --- | --- | --- | --- | --- | --- | --- |
| 128 | 9 | jellyfish | random regular graph; degree budget 9 | 574 | 4 | 2.44 |
| 128 | 9 | flatbutterfly | 2-ary 7-flat; degree 7 | 448 | 7 | 3.53 |
| 128 | 9 | dragonfly | 16 groups x 8 nodes; 2 global port(s) per node | 576 | 3 | 2.63 |
| 128 | 9 | megafly | 8 groups x [8 leaf + 8 spine] | 540 | 5 | 3.46 |
| 128 | 12 | jellyfish | random regular graph; degree budget 12 | 768 | 3 | 2.18 |
| 128 | 12 | flatbutterfly | 2-ary 7-flat; degree 7 | 448 | 7 | 3.53 |
| 128 | 12 | dragonfly | 16 groups x 8 nodes; 5 global port(s) per node | 766 | 3 | 2.32 |
| 128 | 12 | megafly | 8 groups x [8 leaf + 8 spine] | 540 | 5 | 3.46 |
| 256 | 12 | jellyfish | random regular graph; degree budget 12 | 1536 | 4 | 2.51 |
| 256 | 12 | flatbutterfly | 4-ary 4-flat; degree 12 | 1536 | 4 | 3.01 |
| 256 | 12 | dragonfly | 32 groups x 8 nodes; 5 global port(s) per node | 1536 | 3 | 2.62 |
| 256 | 12 | megafly | 16 groups x [8 leaf + 8 spine] | 1144 | 5 | 3.55 |

### Full MDP, makespan ms (per-class TP / DP in brackets)

| nodes/deg | AllReduce | transport | Jelly | FB | Dfly | Mfly | dheu | best static | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 128/9 | rhd | roce | 31.39 (12.89 / 31.39) | 8.13 (1.02 / 8.13) | 47.22 (5.31 / 47.22) | 157.10 (82.91 / 157.10) | 24.81 (8.06 / 24.81) | FB | 3.86x |
| 128/9 | rhd | dctcp | 30.40 (7.48 / 30.40) | 8.06 (1.01 / 8.06) | 45.98 (1.72 / 45.98) | 133.08 (12.68 / 133.08) | 7.68 (4.64 / 7.68) | FB | 3.77x |
| 128/9 | ring | roce | 35.52 (13.55 / 35.52) | 16.22 (2.04 / 16.22) | 64.81 (6.08 / 64.81) | 153.57 (75.07 / 153.57) | 23.52 (8.39 / 23.52) | FB | 2.19x |
| 128/9 | ring | dctcp | 33.11 (8.47 / 33.11) | 16.13 (2.02 / 16.13) | 64.44 (1.92 / 64.44) | 136.38 (12.79 / 136.38) | 8.26 (7.13 / 8.26) | FB | 2.05x |
| 128/12 | rhd | roce | 35.89 (14.41 / 35.89) | 8.13 (1.02 / 8.13) | 33.19 (2.15 / 33.19) | 157.10 (82.91 / 157.10) | 19.35 (5.81 / 19.35) | FB | 4.08x |
| 128/12 | rhd | dctcp | 33.47 (7.29 / 33.47) | 8.06 (1.01 / 8.06) | 33.62 (1.63 / 33.62) | 133.08 (12.68 / 133.08) | 7.87 (5.04 / 7.87) | FB | 4.15x |
| 256/12 | rhd | roce | 38.81 (16.01 / 38.81) | 8.13 (1.02 / 8.13) | 17.48 (3.22 / 17.48) | 296.59 (115.88 / 296.59) | 26.15 (6.95 / 26.15) | FB | 2.15x |
| 256/12 | rhd | dctcp | 36.34 (8.13 / 36.34) | 8.06 (1.01 / 8.06) | 17.52 (2.24 / 17.52) | 271.41 (31.24 / 271.41) | 10.88 (5.97 / 10.88) | FB | 2.17x |
| 256/12 | ring | roce | 57.13 (25.06 / 57.13) | 8.09 (1.02 / 8.09) | 17.04 (3.64 / 17.04) | 359.10 (187.63 / 359.10) | - | FB | 2.11x |
| 256/12 | ring | dctcp | 51.82 (10.12 / 51.82) | 8.06 (1.01 / 8.06) | 16.90 (2.35 / 16.90) | 319.11 (31.36 / 319.11) | - | FB | 2.10x |

### Tenancy: job 0 with 2 / 4 tenants divided by job 0 alone

| nodes/deg | transport | placement | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 128/9 | roce | compact | 1.03 / 1.10 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.17 / 1.37 |
| 128/9 | roce | scatter | 1.13 / 1.65 | 1.30 / 2.67 | 1.41 / 2.23 | 2.10 / 3.75 | 1.16 / 1.43 |
| 128/9 | dctcp | compact | 1.01 / 1.08 | 1.00 / 1.00 | 1.00 / 1.01 | 1.02 / 1.04 | 0.94 / 1.13 |
| 128/9 | dctcp | scatter | 1.07 / 1.39 | 1.19 / 2.79 | 1.49 / 1.90 | 1.73 / 2.99 | 1.12 / 2.05 |
| 256/12 | roce | compact | 1.43 / 1.78 | 1.00 / 1.00 | 0.97 / 0.99 | 1.00 / 1.65 | - |
| 256/12 | roce | scatter | 1.22 / 1.68 | 1.11 / 1.41 | 1.31 / 1.93 | 1.76 / 3.58 | - |
| 256/12 | dctcp | compact | 1.42 / 1.77 | 1.00 / 1.00 | 0.99 / 1.00 | 1.00 / 1.76 | - |
| 256/12 | dctcp | scatter | 1.09 / 1.45 | 1.16 / 1.43 | 1.33 / 1.88 | 1.62 / 3.63 | - |

### Tenancy: job 0 alone, makespan ms

| nodes/deg | transport | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- |
| 128/9 | roce | 32.51 | 8.13 | 58.88 | 50.05 | 18.69 |
| 128/9 | dctcp | 32.41 | 8.06 | 56.33 | 48.88 | 5.04 |
| 256/12 | roce | 23.97 | 8.13 | 45.79 | 163.07 | - |
| 256/12 | dctcp | 23.43 | 8.06 | 48.09 | 130.61 | - |

Non-OK runs: 0

phased
======

### Full MDP: makespan ms by iteration mode

**A: 16 nodes / 4 ports, TP2 DP2 PP2 EP2, roce**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 19.33 | 5.42 | 21.74 | 34.50 | 11.55 | FB < Jelly < Dfly < Mfly | 3.57x |
| ph4c0 | 20.01 | 7.23 | 24.09 | 35.96 | 16.54 | FB < Jelly < Dfly < Mfly | 2.77x |
| ph8c0 | 19.90 | 7.04 | 23.77 | 35.79 | 16.41 | FB < Jelly < Dfly < Mfly | 2.83x |
| ph8c100 | 21.81 | 9.74 | 26.26 | 37.52 | 18.39 | FB < Jelly < Dfly < Mfly | 2.24x |

**A: 16 nodes / 4 ports, TP2 DP2 PP2 EP2, dctcp**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 34.18 | 5.37 | 31.05 | 52.29 | 24.11 | FB < Dfly < Jelly < Mfly | 5.78x |
| ph4c0 | 138.37 | 7.17 | 58.34 | 56.67 | 182.25 | FB < Mfly < Dfly < Jelly | 7.90x |
| ph8c0 | 30.38 | 7.00 | 47.34 | 49.31 | 39.25 | FB < Jelly < Dfly < Mfly | 4.34x |
| ph8c100 | 32.57 | 9.70 | 50.34 | 61.29 | 31.28 | FB < Jelly < Dfly < Mfly | 3.36x |

**B: 128 nodes / 9 ports, TP4 DP4 PP4 EP2, roce**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 31.39 | 8.13 | 47.22 | 157.10 | 24.81 | FB < Jelly < Dfly < Mfly | 3.86x |
| ph8c0 | 33.62 | 10.70 | 52.04 | 151.71 | 31.74 | FB < Jelly < Dfly < Mfly | 3.14x |
| ph8c100 | 36.13 | 14.00 | 54.73 | 152.78 | 34.77 | FB < Jelly < Dfly < Mfly | 2.58x |

**B: 128 nodes / 9 ports, TP4 DP4 PP4 EP2, dctcp**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 53.69 | 8.06 | 55.70 | 152.81 | 34.24 | FB < Jelly < Dfly < Mfly | 6.66x |
| ph8c0 | 87.00 | 10.65 | 171.18 | 273.28 | 312.35 | FB < Jelly < Dfly < Mfly | 8.17x |
| ph8c100 | 65.78 | 13.95 | 162.85 | 238.31 | 200.41 | FB < Jelly < Dfly < Mfly | 4.72x |

**C: 256 nodes / 12 ports, TP4 DP4 PP4 EP4, roce**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 38.81 | 8.13 | 17.48 | 296.59 | NA | FB < Dfly < Jelly < Mfly | 2.15x |
| ph8c0 | 39.40 | 10.08 | 22.93 | 305.29 | NA | FB < Dfly < Jelly < Mfly | 2.28x |
| ph8c100 | 41.58 | 13.38 | 25.60 | 305.17 | NA | FB < Dfly < Jelly < Mfly | 1.91x |

**C: 256 nodes / 12 ports, TP4 DP4 PP4 EP4, dctcp**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 63.75 | 8.06 | 40.03 | 264.71 | NA | FB < Dfly < Jelly < Mfly | 4.97x |
| ph8c0 | 109.64 | 10.03 | 342.69 | 505.43 | NA | FB < Jelly < Dfly < Mfly | 10.93x |
| ph8c100 | 88.79 | 13.33 | 342.69 | 474.82 | NA | FB < Jelly < Dfly < Mfly | 6.66x |

### Tenancy, phased: job 0 with 2 / 4 tenants divided by job 0 alone (concurrent in brackets)

| series | transport | placement | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| D (128 nodes) | roce | compact | 1.04/1.12 (conc 1.03/1.10) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.01/1.00 (conc 1.00/1.00) | 1.15/1.26 (conc 1.17/1.37) |
| D (128 nodes) | roce | scatter | 1.12/1.63 (conc 1.13/1.65) | 1.25/2.67 (conc 1.30/2.67) | 1.23/2.03 (conc 1.41/2.23) | 2.05/3.48 (conc 2.10/3.75) | 1.12/1.43 (conc 1.16/1.43) |
| D (128 nodes) | dctcp | compact | 1.23/1.87 (conc 1.00/1.06) | 1.00/1.00 (conc 1.00/1.00) | 1.00/0.88 (conc 1.00/1.13) | 1.24/0.93 (conc 0.94/0.96) | 0.68/1.00 (conc 0.74/1.04) |
| D (128 nodes) | dctcp | scatter | 1.27/1.53 (conc 1.00/0.96) | 1.10/2.25 (conc 1.12/1.76) | 1.35/1.81 (conc 1.54/1.72) | 1.76/3.66 (conc 1.45/2.51) | 1.78/2.94 (conc 1.01/0.96) |
| E (16 nodes) | roce | compact | 2.01/2.00 (conc 2.00/2.00) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.12/1.22 (conc 1.11/1.19) |
| E (16 nodes) | roce | scatter | 1.00/1.00 (conc 1.00/1.00) | 1.09/1.65 (conc 1.00/1.26) | 1.04/1.07 (conc 1.01/1.06) | 1.90/3.58 (conc 2.12/3.40) | 1.15/1.48 (conc 1.17/1.42) |
| E (16 nodes) | dctcp | compact | 1.12/2.04 (conc 1.36/1.36) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.22/1.11 (conc 1.16/1.16) |
| E (16 nodes) | dctcp | scatter | 1.00/1.00 (conc 1.00/1.00) | 1.06/4.63 (conc 1.00/2.03) | 1.90/1.98 (conc 1.03/1.02) | 1.62/2.59 (conc 1.31/1.69) | 1.06/1.12 (conc 1.01/1.04) |

Non-OK runs: 0

seeds
=====

### S1. LOGICAL_OCS at 16 nodes across seeds (seed 1 = the stored rows)

| transport | seed | Dragonfly best static | Dragonfly within 1% of best | dheu vs best static (mean) | dheu wins |
| --- | --- | --- | --- | --- | --- |
| roce | seed1 | 4/4 | 4/4 | 0.73x (0.62-0.85) | 0/4 |
| roce | seed2 | 3/4 | 4/4 | 0.75x (0.66-0.85) | 0/4 |
| roce | seed3 | 4/4 | 4/4 | 0.74x (0.65-0.85) | 0/4 |
| roce | seed4 | 4/4 | 4/4 | 0.71x (0.59-0.84) | 0/4 |
| roce | seed5 | 3/4 | 3/4 | 0.72x (0.64-0.85) | 0/4 |
| dctcp | seed1 | 3/4 | 4/4 | 1.96x (1.43-2.50) | 4/4 |
| dctcp | seed2 | 2/4 | 3/4 | 1.98x (1.43-2.50) | 4/4 |
| dctcp | seed3 | 2/4 | 4/4 | 1.95x (1.45-2.50) | 4/4 |
| dctcp | seed4 | 3/4 | 4/4 | 2.01x (1.44-2.50) | 4/4 |
| dctcp | seed5 | 1/4 | 2/4 | 1.95x (1.44-2.50) | 4/4 |

### S2. OCS_MT_MDP at 16 nodes: interference with 4 tenants (job 0 with 4 / alone)

| transport | placement | arm | seed 13 (stored) | seeds 2-5 |
| --- | --- | --- | --- | --- |
| roce | compact | Jelly | 2.00 | 1.00, 1.00, 2.80, 1.94 |
| roce | compact | FB | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | Dfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | Mfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | dheu | 1.19 | 1.36, 1.34, 1.26, 1.32 |
| roce | scatter | Jelly | 1.00 | 2.00, 1.11, 2.43, 2.31 |
| roce | scatter | FB | 1.26 | 2.07, 3.02, 1.00, 2.09 |
| roce | scatter | Dfly | 1.06 | 2.87, 1.89, 1.44, 2.14 |
| roce | scatter | Mfly | 3.40 | 3.60, 4.23, 4.62, 1.89 |
| roce | scatter | dheu | 1.42 | 1.25, 1.29, 1.39, 1.22 |
| dctcp | compact | Jelly | 1.99 | 1.00, 1.00, 2.78, 1.91 |
| dctcp | compact | FB | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| dctcp | compact | Dfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| dctcp | compact | Mfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| dctcp | compact | dheu | 1.60 | 1.59, 1.31, 1.59, 1.66 |
| dctcp | scatter | Jelly | 1.00 | 2.00, 1.11, 1.48, 2.05 |
| dctcp | scatter | FB | 1.03 | 2.12, 2.91, 1.00, 2.12 |
| dctcp | scatter | Dfly | 1.07 | 2.82, 1.88, 1.45, 2.16 |
| dctcp | scatter | Mfly | 3.78 | 3.77, 4.20, 4.69, 1.88 |
| dctcp | scatter | dheu | 1.31 | 1.17, 1.33, 1.53, 1.22 |

### S2/S3. Full MDP across seeds: makespan ms (min-max over seeds 2-5) and the winner

| series | transport | Jelly | FB | Dfly | Mfly | dheu | winner in every seed? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S2fullmdp | roce | 9.84-18.23 | 5.42-5.42 | 21.74-21.74 | 34.50-34.50 | 12.70-14.50 | yes, FB |
| S2fullmdp | dctcp | 8.74-17.86 | 5.37-5.37 | 21.48-21.48 | 33.56-33.56 | 5.25-5.83 | yes, FB |
| S3fullmdp | roce | 27.74-50.35 | 8.13-8.13 | 47.22-47.22 | 157.10-157.10 | - | yes, FB |
| S3fullmdp | dctcp | 28.25-48.03 | 8.06-8.06 | 45.98-45.98 | 133.08-133.08 | - | yes, FB |

### S3. 128 nodes, 4 tenants / alone, per seed

| transport | placement | Jelly | FB | Dfly | Mfly |
| --- | --- | --- | --- | --- | --- |
| roce | compact | 1.85, 1.39, 1.50, 1.75 | 1.00, 1.00, 1.00, 1.00 | 1.00, 1.00, 1.00, 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | scatter | 1.68, 1.45, 1.90, 1.97 | 3.49, 2.06, 3.36, 3.64 | 1.57, 1.40, 2.36, 2.18 | 3.09, 2.60, 3.05, 3.40 |
| dctcp | compact | 2.29, 1.31, 1.36, 1.46 | 1.00, 1.00, 1.00, 1.00 | 1.01, 1.01, 1.01, 1.01 | 1.04, 1.04, 1.04, 1.04 |
| dctcp | scatter | 1.57, 1.52, 1.92, 2.17 | 3.28, 1.69, 3.00, 3.11 | 1.56, 1.32, 2.43, 2.23 | 2.50, 2.08, 2.65, 2.69 |

Non-OK runs: 0

432
===

### Fabrics as built (432 nodes, 16 ports)

| arm | construction | links | diameter | mean hops |
| --- | --- | --- | --- | --- |
| dragonfly | 36 groups x 12 nodes; 5 global port(s) per node | 3455 | 3 | 2.67 |
| jellyfish | random regular graph; degree budget 16 | 3455 | 4 | 2.50 |
| megafly | 16 groups x [14 leaf + 13 spine] | 3032 | 5 | 3.64 |

### Full MDP, makespan ms (TP-class / DP-class)

| decomposition | transport | Jelly | Dfly | Mfly | wall s / RSS GB (max) |
| --- | --- | --- | --- | --- | --- |
| TP4 DP9 PP4 EP3 | roce | 53.85 (18.22 / 53.85) | 106.13 (4.01 / 106.13) | 657.62 (453.84 / 657.62) | 36 / 1.33 |
| TP4 DP9 PP4 EP3 | dctcp | 96.26 (69.85 / 96.26) | 191.57 (1.38 / 191.57) | 731.78 (160.91 / 731.78) | 14 / 0.17 |
| TP8 DP6 PP9 EP1 | roce | 41.42 (12.78 / 41.42) | 99.08 (5.64 / 99.08) | 539.58 (313.32 / 539.58) | 31 / 1.23 |
| TP8 DP6 PP9 EP1 | dctcp | 72.48 (58.55 / 72.48) | 145.63 (33.71 / 145.63) | 508.03 (155.85 / 508.03) | 15 / 0.17 |

### Tenancy, 108-node jobs: job 0 with 2 / 4 tenants divided by job 0 alone (alone ms in brackets)

| transport | placement | Jelly | Dfly | Mfly |
| --- | --- | --- | --- | --- |
| roce | compact | 1.44 / 1.50 (31.9) | 1.00 / 1.00 (79.1) | 0.99 / 1.81 (300.1) |
| roce | scatter | 1.46 / 2.73 (17.1) | 1.25 / 1.75 (28.8) | 2.07 / 3.92 (155.8) |
| dctcp | compact | 1.01 / 1.11 (58.2) | 0.96 / 0.93 (88.6) | 0.95 / 1.66 (249.8) |
| dctcp | scatter | 1.04 / 1.56 (35.1) | 0.88 / 1.10 (49.1) | 1.75 / 3.22 (169.0) |

Non-OK runs: 0

dheuscale
=========

### 256 nodes, one full-MDP job (TP4 DP4 PP4 EP4, RHD), degree 12, ms

| arm | RoCE | DCTCP (exact queue) |
| --- | --- | --- |
| jellyfish | 38.81 | 63.75 |
| flatbutterfly | 8.13 | 8.06 |
| dragonfly | 17.48 | 40.03 |
| megafly | 296.59 | 264.71 |
| dheu, RoCE demand invisible (OCS_SCALE.md) | 26.15 | - |
| **dheu, demand visible (this sweep)** | **5.52** | **34.40** |

### 432 nodes, one full-MDP job, degree 16, ms (static families from OCS_432.md)

| decomposition | transport | Jellyfish | Dragonfly | Megafly | dheu (demand visible) | best static / dheu |
| --- | --- | --- | --- | --- | --- | --- |
| A tp4.dp9.pp4.ep3 | roce | 53.85 | 106.13 | 657.62 | 11.44 | 4.71 |
| A tp4.dp9.pp4.ep3 | dctcp | 96.26 | 191.57 | 731.78 | 71.04 | 1.36 |
| B tp8.dp6.pp9.ep1 | roce | 41.42 | 99.08 | 539.58 | 9.26 | 4.48 |
| B tp8.dp6.pp9.ep1 | dctcp | 72.48 | 145.63 | 508.03 | 55.38 | 1.31 |

### 432 nodes, 1/2/4 jobs of 108 nodes (TP4 DP3 PP3 EP3): job 0's time, ms, and isolation (4 jobs / alone)

| transport | placement | arm | alone | 2 jobs | 4 jobs | 4 jobs / alone |
| --- | --- | --- | --- | --- | --- | --- |
| roce | compact | jellyfish | 31.86 | 45.78 | 47.86 | 1.50 |
| roce | compact | dragonfly | 79.10 | 79.11 | 79.12 | 1.00 |
| roce | compact | megafly | 300.07 | 295.77 | 543.93 | 1.81 |
| roce | compact | dheu | 2.84 | 3.48 | 4.12 | 1.45 |
| roce | scatter | jellyfish | 17.06 | 24.90 | 46.59 | 2.73 |
| roce | scatter | dragonfly | 28.76 | 35.95 | 50.32 | 1.75 |
| roce | scatter | megafly | 155.81 | 322.24 | 610.84 | 3.92 |
| roce | scatter | dheu | 2.88 | 3.36 | 5.57 | 1.94 |
| dctcp | compact | jellyfish | 58.24 | 58.98 | 64.80 | 1.11 |
| dctcp | compact | dragonfly | 88.62 | 85.24 | 82.61 | 0.93 |
| dctcp | compact | megafly | 249.75 | 238.35 | 415.43 | 1.66 |
| dctcp | compact | dheu | 23.35 | 34.07 | 33.90 | 1.45 |
| dctcp | scatter | jellyfish | 35.15 | 36.65 | 54.78 | 1.56 |
| dctcp | scatter | dragonfly | 49.08 | 43.40 | 54.01 | 1.10 |
| dctcp | scatter | megafly | 169.00 | 294.98 | 544.79 | 3.22 |
| dctcp | scatter | dheu | 33.80 | 33.88 | 33.21 | 0.98 |

### Wall time of a demand-driven run, before and after the live-link drain census

| run | before | after |
| --- | --- | --- |
| 256 nodes, full MDP, roce | 1979 s | 6.5 s |
| 256 nodes, full MDP, dctcp | 1740 s | 7.3 s |

framework
=========


### A: one 16-node job (TP2 DP2 PP2 EP2), 4 ports; iteration time, ms

| transport | arm | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- | --- |
| roce | jellyfish | 19.33 | 19.90 | 21.81 | 25.91 | 27.58 |
| roce | flatbutterfly | 5.42 | 7.04 | 9.74 | 7.47 | 10.02 |
| roce | dragonfly | 21.74 | 23.77 | 26.26 | 25.25 | 27.83 |
| roce | megafly | 34.50 | 35.79 | 37.52 | 35.97 | 38.24 |
| roce | dheu | 9.06 | 9.86 | 11.31 | 12.11 | 13.97 |
| dctcp | jellyfish | 34.18 | 30.38 | 32.57 | 95.69 | 98.14 |
| dctcp | flatbutterfly | 5.37 | 7.00 | 9.70 | 14.35 | 16.35 |
| dctcp | dragonfly | 31.05 | 47.34 | 50.34 | 64.15 | 67.76 |
| dctcp | megafly | 52.29 | 49.31 | 61.29 | 100.65 | 88.80 |
| dctcp | dheu | 24.11 | 39.25 | 31.28 | 83.54 | 96.30 |

**A: best static family, and the demand-driven fabric against it, per mode**

| transport | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | FB > Jelly > Dfly > Mfly; dheu 0.60x | FB > Jelly > Dfly > Mfly; dheu 0.71x | FB > Jelly > Dfly > Mfly; dheu 0.86x | FB > Dfly > Jelly > Mfly; dheu 0.62x | FB > Jelly > Dfly > Mfly; dheu 0.72x |
| dctcp | FB > Dfly > Jelly > Mfly; dheu 0.22x | FB > Jelly > Dfly > Mfly; dheu 0.18x | FB > Jelly > Dfly > Mfly; dheu 0.31x | FB > Dfly > Jelly > Mfly; dheu 0.17x | FB > Dfly > Mfly > Jelly; dheu 0.17x |

### B: one 128-node job (TP4 DP4 PP4 EP2), 9 ports; iteration time, ms

| transport | arm | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- | --- |
| roce | jellyfish | 31.39 | 33.62 | 36.13 | 44.62 | 46.22 |
| roce | flatbutterfly | 8.13 | 10.70 | 14.00 | 10.98 | 13.55 |
| roce | dragonfly | 47.22 | 52.04 | 54.73 | 61.26 | 64.30 |
| roce | megafly | 157.10 | 151.71 | 152.78 | 211.41 | 213.52 |
| roce | dheu | 4.27 | 16.76 | 18.25 | 29.53 | 31.02 |
| dctcp | jellyfish | 53.69 | 87.00 | 65.78 | 195.02 | 192.71 |
| dctcp | flatbutterfly | 8.06 | 10.65 | 13.95 | 17.39 | 21.01 |
| dctcp | dragonfly | 55.70 | 171.18 | 162.85 | 281.02 | 307.43 |
| dctcp | megafly | 152.81 | 273.28 | 238.31 | 355.19 | 386.87 |
| dctcp | dheu | 34.24 | 312.35 | 200.41 | 391.17 | 463.18 |

**B: best static family, and the demand-driven fabric against it, per mode**

| transport | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | FB > Jelly > Dfly > Mfly; dheu 1.90x | FB > Jelly > Dfly > Mfly; dheu 0.64x | FB > Jelly > Dfly > Mfly; dheu 0.77x | FB > Jelly > Dfly > Mfly; dheu 0.37x | FB > Jelly > Dfly > Mfly; dheu 0.44x |
| dctcp | FB > Jelly > Dfly > Mfly; dheu 0.24x | FB > Jelly > Dfly > Mfly; dheu 0.03x | FB > Jelly > Dfly > Mfly; dheu 0.07x | FB > Jelly > Dfly > Mfly; dheu 0.04x | FB > Jelly > Dfly > Mfly; dheu 0.05x |

### D: tenancy on 128 nodes - job 0 with three neighbours / alone, per iteration mode

| transport | placement | arm | concurrent | GPipe 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | compact | jellyfish | 1.10 (32.51) | 1.12 (33.36) | 1.08 (39.63) |
| roce | compact | flatbutterfly | 1.00 (8.13) | 1.00 (9.75) | NA (NA) |
| roce | compact | dragonfly | 1.00 (58.88) | 1.00 (60.38) | NA (NA) |
| roce | compact | megafly | 1.00 (50.05) | 1.00 (51.68) | NA (NA) |
| roce | compact | dheu | 0.98 (3.39) | 1.41 (6.56) | NA (NA) |
| roce | scatter | jellyfish | 1.65 (16.95) | 1.63 (18.07) | NA (17.56) |
| roce | scatter | flatbutterfly | 2.67 (25.33) | 2.67 (25.49) | NA (NA) |
| roce | scatter | dragonfly | 2.23 (15.79) | 2.03 (19.00) | NA (NA) |
| roce | scatter | megafly | 3.75 (51.24) | 3.48 (52.50) | NA (NA) |
| roce | scatter | dheu | 1.62 (2.25) | 1.50 (6.56) | NA (NA) |
| dctcp | compact | jellyfish | 1.06 (45.42) | 1.87 (44.08) | NA (NA) |
| dctcp | compact | flatbutterfly | 1.00 (8.06) | 1.00 (9.69) | NA (NA) |
| dctcp | compact | dragonfly | 1.13 (56.46) | 0.88 (76.96) | NA (NA) |
| dctcp | compact | megafly | 0.96 (92.23) | 0.93 (84.59) | NA (NA) |
| dctcp | compact | dheu | 1.04 (33.33) | 1.00 (101.40) | NA (NA) |
| dctcp | scatter | jellyfish | 0.96 (32.43) | 1.53 (21.20) | NA (NA) |
| dctcp | scatter | flatbutterfly | 1.76 (39.46) | 2.25 (42.90) | NA (NA) |
| dctcp | scatter | dragonfly | 1.72 (21.43) | 1.81 (28.84) | NA (NA) |
| dctcp | scatter | megafly | 2.51 (66.16) | 3.66 (78.29) | NA (NA) |
| dctcp | scatter | dheu | 0.96 (24.33) | 2.94 (38.44) | NA (NA) |

Non-OK OCS runs: 0

pfc
===

### A. LOGICAL_OCS findings under each PFC model (RoCE)

| PFC model | cells | Dragonfly best | Dfly vs Jelly | Dfly vs FB | Dfly vs Mfly | dheu vs best static | dheu wins |
| --- | --- | --- | --- | --- | --- | --- | --- |
| base | 7 | 7/7 | 1.35x (1.05-1.67), 7/7 | 1.61x (1.16-1.92), 7/7 | 1.17x (1.01-1.47), 7/7 | 0.75x (0.62-1.01) | 1/7 |
| delay | 7 | 7/7 | 1.35x (1.05-1.69), 7/7 | 1.61x (1.18-1.92), 7/7 | 1.16x (1.01-1.48), 7/7 | 0.74x (0.62-1.01) | 1/7 |
| xoff8 | 7 | 7/7 | 1.40x (1.08-1.70), 7/7 | 1.61x (1.19-1.92), 7/7 | 1.20x (1.00-1.49), 7/7 | 0.72x (0.62-0.93) | 0/7 |
| xoff32 | 7 | 7/7 | 1.32x (1.03-1.65), 7/7 | 1.61x (1.11-1.92), 7/7 | 1.16x (1.01-1.49), 7/7 | 0.78x (0.63-1.08) | 1/7 |

### A2. Makespan change against the historical model, per arm

| PFC model | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- |
| delay | +0.5% (-0.2 to +1.5) | +0.0% (-0.1 to +0.4) | -0.1% (-0.9 to +0.6) | -0.5% (-3.4 to +0.7) | +0.5% (+0.1 to +1.8) |
| xoff8 | +6.0% (+2.1 to +18.7) | +1.3% (+0.0 to +6.3) | +1.4% (+0.0 to +3.7) | +4.1% (-3.1 to +11.0) | +4.4% (+0.8 to +9.0) |
| xoff32 | -4.0% (-10.4 to -0.5) | -1.8% (-7.3 to +0.0) | -1.6% (-5.4 to +0.0) | -2.2% (-7.3 to +13.4) | -5.1% (-12.8 to -1.3) |

### A3. Does each model hold its own headroom? (all 35 RoCE runs per model)

| PFC model | XOFF | headroom | peak overshoot (pkts) | ingress overflows | peak egress transit (pkts) | transit-only egress overflows |
| --- | --- | --- | --- | --- | --- | --- |
| base | 15 | 5 | 6.21 | 281 | 71.0 | 0 |
| delay | 15 | 5 | 10.06 | 16697 | 74.4 | 0 |
| xoff8 | 8 | 5 | 9.99 | 48973 | 46.3 | 0 |
| xoff32 | 32 | 5 | 9.99 | 19431 | 143.1 | 0 |

### A4. Where the 'LOSSLESS not working' warnings come from (dheu, base model)

| workload | ingress overflows | egress overflows, host-originated | egress overflows, transit | of which transit alone over the limit | peak egress transit (pkts of 200) |
| --- | --- | --- | --- | --- | --- |
| dlrm16 | 9 | 629046 | 75029 | 0 | 52.3 |
| vgg16 | 53 | 51348 | 7272 | 0 | 52.2 |
| transformer16 | 15 | 544505 | 65681 | 0 | 53.2 |
| candle16 | 64 | 1343992 | 155238 | 0 | 52.6 |
| dlrm32 | 24 | 1763655 | 196559 | 0 | 69.3 |
| vgg32 | 105 | 122019 | 24321 | 0 | 68.8 |
| transformer32 | 11 | 3208884 | 430782 | 0 | 65.6 |

### B. Shortest-path routing + dheu under PFC: does it still deadlock?

| PFC model | dlrm16 | vgg16 | transformer16 | candle16 | dlrm32 | vgg32 | transformer32 | deadlocked |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| base | **deadlock** (81/1278) | **deadlock** (767/768) | OK 132.2 ms | **deadlock** (56/1580) | **deadlock** (256/3662) | **deadlock** (1474/1504) | **deadlock** (7789/7878) | 6/7 |
| delay | **deadlock** (81/1278) | **deadlock** (767/768) | OK 132.3 ms | **deadlock** (56/1580) | **deadlock** (256/3662) | **deadlock** (1474/1504) | **deadlock** (7789/7878) | 6/7 |
| xoff8 | **deadlock** (81/1278) | **deadlock** (767/768) | OK 134.5 ms | **deadlock** (56/1580) | **deadlock** (256/3662) | **deadlock** (1474/1504) | **deadlock** (7790/7878) | 6/7 |
| xoff32 | **deadlock** (81/1278) | OK 383.3 ms | OK 128.3 ms | **deadlock** (56/1580) | **deadlock** (256/3662) | **deadlock** (1488/1504) | **deadlock** (7789/7878) | 5/7 |

### C. OCS_MT_MDP findings under each PFC model (RoCE)

Interference: job 0's makespan with 2 and 4 tenants divided by job 0 alone.

| PFC model | placement | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- |
| base | compact | 2.00 / 2.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| base | scatter | 1.00 / 1.00 | 1.00 / 1.26 | 1.01 / 1.06 | 2.12 / 3.40 | 1.17 / 1.42 |
| delay | compact | 2.00 / 2.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| delay | scatter | 1.00 / 1.00 | 1.00 / 1.26 | 1.01 / 1.05 | 2.12 / 3.40 | 1.17 / 1.42 |
| xoff8 | compact | 2.02 / 2.02 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.12 / 1.27 |
| xoff8 | scatter | 1.00 / 1.00 | 1.00 / 1.15 | 1.07 / 1.12 | 1.96 / 3.38 | 1.15 / 1.45 |
| xoff32 | compact | 2.00 / 2.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| xoff32 | scatter | 1.00 / 1.00 | 1.00 / 1.23 | 1.03 / 1.06 | 1.91 / 3.77 | 1.17 / 1.41 |

Full MDP (16-node TP2 DP2 PP2 EP2 job), makespan ms and the hypercube's lead:

| PFC model | Jelly | FB | Dfly | Mfly | dheu | FB lead over the static families | FB lead over dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| base | 19.33 | 5.42 | 21.74 | 34.50 | 11.55 | 3.57-6.37x | 2.13x |
| delay | 19.39 | 5.42 | 21.75 | 34.49 | 11.60 | 3.58-6.36x | 2.14x |
| xoff8 | 21.48 | 5.42 | 22.62 | 34.99 | 12.20 | 3.97-6.46x | 2.25x |
| xoff32 | 19.30 | 5.42 | 21.67 | 33.85 | 11.42 | 3.56-6.25x | 2.11x |
