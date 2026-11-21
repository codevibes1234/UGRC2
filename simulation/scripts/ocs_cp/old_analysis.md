
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
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | 9.57 | 3.97 | 0.42 | 5.63 (FB) | 1.42 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | 10.63 | 5.40 | 0.51 | 5.63 (FB) | 1.04 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | 11.37 | 5.54 | 0.49 | 5.63 (FB) | 1.02 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | 8.03 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | 9.37 | 2.87 | 0.31 | 6.28 (FB) | 2.19 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | 11.42 | 3.17 | 0.28 | 6.75 (Jelly) | 2.13 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | 11.55 | 8.26 | 0.72 | 5.63 (FB) | 0.68 |

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
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | base | 9.57 | 3.97 | 0.42 | 5.63 (FB) | 1.42 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | base | 10.63 | 5.40 | 0.51 | 5.63 (FB) | 1.04 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | base | 11.37 | 5.54 | 0.49 | 5.63 (FB) | 1.02 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | base | 8.03 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | base | 9.37 | 2.87 | 0.31 | 6.28 (FB) | 2.19 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | base | 11.42 | 3.17 | 0.28 | 6.75 (Jelly) | 2.13 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | base | 11.55 | 8.26 | 0.72 | 5.63 (FB) | 0.68 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | delay | 9.57 | 4.10 | 0.43 | 5.63 (FB) | 1.37 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | delay | 10.63 | 5.40 | 0.51 | 5.63 (FB) | 1.04 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | delay | 11.38 | 5.49 | 0.48 | 5.63 (FB) | 1.03 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | delay | 8.03 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | delay | 9.37 | 2.87 | 0.31 | 6.28 (FB) | 2.19 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | delay | 11.39 | 3.16 | 0.28 | 6.74 (Jelly) | 2.14 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | delay | 11.60 | 8.70 | 0.75 | 5.63 (FB) | 0.65 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 9.54 | 3.85 | 0.40 | 5.63 (FB) | 1.46 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 10.68 | 5.20 | 0.49 | 5.63 (FB) | 1.08 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 12.11 | 5.21 | 0.43 | 5.63 (FB) | 1.08 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 8.09 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 9.34 | 3.01 | 0.32 | 6.28 (FB) | 2.09 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | xoff8 | 11.69 | 4.17 | 0.36 | 6.74 (Jelly) | 1.62 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | xoff8 | 12.20 | 11.13 | 0.91 | 5.63 (FB) | 0.51 |
| tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 9.59 | 3.98 | 0.42 | 5.63 (FB) | 1.41 |
| tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 10.60 | 5.40 | 0.51 | 5.63 (FB) | 1.04 |
| tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 11.38 | 5.45 | 0.48 | 5.63 (FB) | 1.03 |
| tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 8.03 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 9.39 | 2.85 | 0.30 | 6.28 (FB) | 2.20 |
| tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | xoff32 | 11.36 | 3.11 | 0.27 | 6.74 (Jelly) | 2.17 |
| fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | xoff32 | 11.42 | 5.60 | 0.49 | 5.63 (FB) | 1.00 |

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
| F16d4 | 16 | 4 | 16 | tp2.dp2.pp2.ep2 | rhd | 11.55 | 8.26 | 0.72 | 5.63 (FB) | 0.68 |
| F16d4 | 16 | 4 | 16 | tp2.dp2.pp2.ep2 | ring | 11.55 | 8.26 | 0.72 | 5.63 (FB) | 0.68 |
| F16d4 | 16 | 4 | 16 | tp4.dp2.pp2.ep1 | rhd | 11.39 | 6.21 | 0.55 | 5.63 (FB) | 0.91 |
| F16d4 | 16 | 4 | 16 | tp4.dp2.pp2.ep1 | ring | 11.99 | 6.06 | 0.51 | 5.63 (FB) | 0.93 |
| F16d4 | 16 | 4 | 16 | tp4.dp4.pp1.ep1 | rhd | 19.67 | 8.81 | 0.45 | 8.44 (FB) | 0.96 |
| F16d4 | 16 | 4 | 16 | tp4.dp4.pp1.ep1 | ring | 19.56 | 10.62 | 0.54 | 16.22 (FB) | 1.53 |
| F16d4 | 16 | 4 | 16 | tp8.dp2.pp1.ep1 | rhd | 12.04 | 6.12 | 0.51 | 5.63 (FB) | 0.92 |
| F16d4 | 16 | 4 | 16 | tp8.dp2.pp1.ep1 | ring | 12.10 | 6.30 | 0.52 | 5.63 (FB) | 0.89 |
| F16d4 | 16 | 4 | 16 | tp16.dp1.pp1.ep1 | rhd | 3.03 | 3.41 | 1.13 | 1.33 (FB) | 0.39 |
| F16d4 | 16 | 4 | 16 | tp16.dp1.pp1.ep1 | ring | 3.63 | 2.11 | 0.58 | 2.64 (Dfly) | 1.25 |
| F16d6 | 16 | 6 | 16 | tp2.dp2.pp2.ep2 | rhd | 9.35 | 3.29 | 0.35 | 5.63 (FB) | 1.71 |
| F16d6 | 16 | 6 | 16 | tp2.dp2.pp2.ep2 | ring | 9.35 | 3.29 | 0.35 | 5.63 (FB) | 1.71 |
| F16d6 | 16 | 6 | 16 | tp4.dp2.pp2.ep1 | rhd | 9.10 | 2.69 | 0.30 | 5.63 (FB) | 2.09 |
| F16d6 | 16 | 6 | 16 | tp4.dp2.pp2.ep1 | ring | 9.23 | 4.38 | 0.47 | 5.63 (FB) | 1.29 |
| F16d6 | 16 | 6 | 16 | tp4.dp4.pp1.ep1 | rhd | 14.04 | 4.67 | 0.33 | 8.44 (FB) | 1.81 |
| F16d6 | 16 | 6 | 16 | tp4.dp4.pp1.ep1 | ring | 14.04 | 6.65 | 0.47 | 8.09 (FB) | 1.22 |
| F16d6 | 16 | 6 | 16 | tp8.dp2.pp1.ep1 | rhd | 8.91 | 3.38 | 0.38 | 5.63 (FB) | 1.67 |
| F16d6 | 16 | 6 | 16 | tp8.dp2.pp1.ep1 | ring | 9.48 | 3.48 | 0.37 | 5.63 (FB) | 1.62 |
| F16d6 | 16 | 6 | 16 | tp16.dp1.pp1.ep1 | rhd | 2.63 | 2.89 | 1.10 | 1.33 (FB) | 0.46 |
| F16d6 | 16 | 6 | 16 | tp16.dp1.pp1.ep1 | ring | 3.36 | 0.74 | 0.22 | 1.65 (Jelly) | 2.24 |
| F27d6 | 27 | 6 | 27 | tp3.dp3.pp3.ep1 | ring | 15.62 | 6.58 | 0.42 | 7.19 (FB) | 1.09 |
| F27d6 | 27 | 6 | 27 | tp9.dp3.pp1.ep1 | ring | 15.73 | 6.29 | 0.40 | 7.19 (FB) | 1.14 |
| F27d6 | 27 | 6 | 27 | tp3.dp9.pp1.ep1 | ring | 22.70 | 9.34 | 0.41 | 19.20 (FB) | 2.06 |
| F27d6 | 27 | 6 | 27 | tp27.dp1.pp1.ep1 | ring | 4.32 | 4.48 | 1.04 | 1.63 (Dfly) | 0.36 |
| F27d6 | 27 | 6 | 27 | tp3.dp3.pp1.ep3 | ring | 16.17 | 7.23 | 0.45 | 7.19 (FB) | 0.99 |
| J12d4 | 16 | 4 | 12 | tp3.dp2.pp2.ep1 | ring | 12.35 | 5.65 | 0.46 | 7.46 (Jelly) | 1.32 |
| J12d4 | 16 | 4 | 12 | tp3.dp2.pp2.ep1 | auto | 12.35 | 5.65 | 0.46 | 7.46 (Jelly) | 1.32 |
| J12d4 | 16 | 4 | 12 | tp2.dp3.pp2.ep1 | ring | 15.70 | 7.95 | 0.51 | 10.80 (FB) | 1.36 |
| J12d4 | 16 | 4 | 12 | tp2.dp3.pp2.ep1 | auto | 15.70 | 7.95 | 0.51 | 10.80 (FB) | 1.36 |
| J12d4 | 16 | 4 | 12 | tp3.dp4.pp1.ep1 | ring | 17.87 | 8.17 | 0.46 | 19.10 (Dfly) | 2.34 |
| J12d4 | 16 | 4 | 12 | tp3.dp4.pp1.ep1 | auto | 18.35 | 6.58 | 0.36 | 14.83 (Jelly) | 2.25 |
| J12d4 | 16 | 4 | 12 | tp4.dp3.pp1.ep1 | ring | 16.78 | 7.67 | 0.46 | 10.80 (FB) | 1.41 |
| J12d4 | 16 | 4 | 12 | tp4.dp3.pp1.ep1 | auto | 16.06 | 7.67 | 0.48 | 10.80 (FB) | 1.41 |
| J12d4 | 16 | 4 | 12 | tp12.dp1.pp1.ep1 | ring | 3.45 | 2.82 | 0.82 | 2.55 (Dfly) | 0.91 |
| J12d4 | 16 | 4 | 12 | tp12.dp1.pp1.ep1 | auto | 3.45 | 2.82 | 0.82 | 2.55 (Dfly) | 0.91 |
| J12d6 | 16 | 6 | 12 | tp3.dp2.pp2.ep1 | ring | 10.18 | 2.39 | 0.23 | 7.20 (Dfly) | 3.01 |
| J12d6 | 16 | 6 | 12 | tp3.dp2.pp2.ep1 | auto | 10.18 | 2.39 | 0.23 | 7.20 (Dfly) | 3.01 |
| J12d6 | 16 | 6 | 12 | tp2.dp3.pp2.ep1 | ring | 11.99 | 5.06 | 0.42 | 7.19 (FB) | 1.42 |
| J12d6 | 16 | 6 | 12 | tp2.dp3.pp2.ep1 | auto | 11.99 | 5.06 | 0.42 | 7.19 (FB) | 1.42 |
| J12d6 | 16 | 6 | 12 | tp3.dp4.pp1.ep1 | ring | 13.46 | 5.52 | 0.41 | 10.09 (Dfly) | 1.83 |
| J12d6 | 16 | 6 | 12 | tp3.dp4.pp1.ep1 | auto | 14.19 | 3.98 | 0.28 | 10.70 (Dfly) | 2.69 |
| J12d6 | 16 | 6 | 12 | tp4.dp3.pp1.ep1 | ring | 12.02 | 5.52 | 0.46 | 7.19 (FB) | 1.30 |
| J12d6 | 16 | 6 | 12 | tp4.dp3.pp1.ep1 | auto | 11.86 | 4.88 | 0.41 | 7.19 (FB) | 1.47 |
| J12d6 | 16 | 6 | 12 | tp12.dp1.pp1.ep1 | ring | 2.49 | 1.28 | 0.51 | 1.30 (Dfly) | 1.01 |
| J12d6 | 16 | 6 | 12 | tp12.dp1.pp1.ep1 | auto | 2.49 | 1.28 | 0.51 | 1.30 (Dfly) | 1.01 |

### PHASED_MDP (`rd_phased.csv`, 31 cells)

| series | nodes | deg | placement | jobs | jobnodes | decomp | variant | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | conc | 11.55 | 8.26 | 0.72 | 5.63 (FB) | 0.68 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph4c0 | 16.54 | 10.49 | 0.63 | 7.50 (FB) | 0.71 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph8c0 | 16.41 | 9.75 | 0.59 | 7.29 (FB) | 0.75 |
| A | 16 | 4 | compact | 1 | 16 | tp2.dp2.pp2.ep2 | ph8c100 | 18.39 | 12.18 | 0.66 | 9.99 (FB) | 0.82 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | conc | 24.81 | 4.86 | 0.20 | 8.44 (FB) | 1.74 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ph8c0 | 31.74 | 18.22 | 0.57 | 11.06 (FB) | 0.61 |
| B | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ph8c100 | 34.77 | 20.35 | 0.59 | 14.36 (FB) | 0.71 |
| D | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | conc | 18.69 | 4.01 | 0.21 | 8.44 (FB) | 2.10 |
| D | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | conc | 21.90 | 3.59 | 0.16 | 8.44 (FB) | 2.35 |
| D | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | conc | 25.57 | 3.81 | 0.15 | 8.44 (FB) | 2.22 |
| D | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | conc | 15.45 | 2.87 | 0.19 | 16.20 (Dfly) | 5.64 |
| D | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | conc | 17.98 | 2.98 | 0.17 | 19.33 (Jelly) | 6.48 |
| D | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | conc | 22.11 | 3.85 | 0.17 | 27.13 (Jelly) | 7.05 |
| D | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 24.05 | 8.72 | 0.36 | 10.10 (FB) | 1.16 |
| D | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 27.63 | 8.87 | 0.32 | 10.10 (FB) | 1.14 |
| D | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 30.35 | 10.49 | 0.35 | 10.10 (FB) | 0.96 |
| D | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 19.11 | 7.63 | 0.40 | 18.54 (Jelly) | 2.43 |
| D | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 21.36 | 9.30 | 0.44 | 20.91 (Jelly) | 2.25 |
| D | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | ph8c0 | 27.38 | 11.58 | 0.42 | 29.05 (Jelly) | 2.51 |
| E | 16 | 4 | compact | 1 | 4 | tp2.dp2.pp1.ep1 | conc | 9.57 | 3.97 | 0.42 | 5.63 (FB) | 1.42 |
| E | 16 | 4 | compact | 2 | 4 | tp2.dp2.pp1.ep1 | conc | 10.63 | 5.40 | 0.51 | 5.63 (FB) | 1.04 |
| E | 16 | 4 | compact | 4 | 4 | tp2.dp2.pp1.ep1 | conc | 11.37 | 5.54 | 0.49 | 5.63 (FB) | 1.02 |
| E | 16 | 4 | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | conc | 8.03 | 2.60 | 0.32 | 6.28 (FB) | 2.41 |
| E | 16 | 4 | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | conc | 9.37 | 2.87 | 0.31 | 6.28 (FB) | 2.19 |
| E | 16 | 4 | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | conc | 11.42 | 3.17 | 0.28 | 6.75 (Jelly) | 2.13 |
| E | 16 | 4 | compact | 1 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 10.06 | 3.95 | 0.39 | 6.34 (FB) | 1.61 |
| E | 16 | 4 | compact | 2 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 11.24 | 5.19 | 0.46 | 6.34 (FB) | 1.22 |
| E | 16 | 4 | compact | 4 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 12.24 | 5.52 | 0.45 | 6.34 (FB) | 1.15 |
| E | 16 | 4 | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 8.94 | 3.57 | 0.40 | 7.09 (Jelly) | 1.98 |
| E | 16 | 4 | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 10.27 | 3.62 | 0.35 | 7.09 (Jelly) | 1.96 |
| E | 16 | 4 | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | ph4c0 | 13.26 | 3.32 | 0.25 | 7.09 (Jelly) | 2.13 |

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
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed2 | 8.32 | 4.04 | 0.49 | 5.63 (FB) | 1.39 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed2 | 9.57 | 5.35 | 0.56 | 5.63 (FB) | 1.05 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed2 | 11.29 | 5.59 | 0.50 | 5.63 (FB) | 1.01 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed2 | 7.50 | 2.98 | 0.40 | 6.20 (FB) | 2.08 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed2 | 8.30 | 3.05 | 0.37 | 6.20 (FB) | 2.03 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed2 | 9.37 | 3.36 | 0.36 | 11.35 (FB) | 3.38 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed2 | 14.14 | 8.51 | 0.60 | 5.63 (FB) | 0.66 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed3 | 8.79 | 3.93 | 0.45 | 5.63 (FB) | 1.43 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed3 | 10.12 | 5.36 | 0.53 | 5.63 (FB) | 1.05 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed3 | 11.80 | 5.29 | 0.45 | 5.63 (FB) | 1.06 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed3 | 9.16 | 2.94 | 0.32 | 6.28 (Jelly) | 2.13 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed3 | 10.44 | 2.82 | 0.27 | 6.28 (Jelly) | 2.23 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed3 | 11.83 | 3.68 | 0.31 | 6.95 (Jelly) | 1.89 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed3 | 12.70 | 7.75 | 0.61 | 5.63 (FB) | 0.73 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed4 | 8.97 | 3.97 | 0.44 | 5.63 (FB) | 1.42 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed4 | 10.05 | 5.39 | 0.54 | 5.63 (FB) | 1.04 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed4 | 11.32 | 5.17 | 0.46 | 5.63 (FB) | 1.09 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed4 | 8.51 | 2.57 | 0.30 | 6.28 (FB) | 2.45 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed4 | 10.39 | 3.05 | 0.29 | 6.28 (FB) | 2.06 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed4 | 11.81 | 3.55 | 0.30 | 6.28 (FB) | 1.77 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed4 | 13.94 | 8.41 | 0.60 | 5.63 (FB) | 0.67 |
| S2tenancy | compact | 1 | 4 | tp2.dp2.pp1.ep1 | seed5 | 10.21 | 4.07 | 0.40 | 5.63 (FB) | 1.38 |
| S2tenancy | compact | 2 | 4 | tp2.dp2.pp1.ep1 | seed5 | 11.18 | 5.18 | 0.46 | 5.63 (FB) | 1.09 |
| S2tenancy | compact | 4 | 4 | tp2.dp2.pp1.ep1 | seed5 | 13.46 | 5.43 | 0.40 | 5.63 (FB) | 1.04 |
| S2tenancy | scatter | 1 | 4 | tp2.dp2.pp1.ep1 | seed5 | 8.04 | 2.58 | 0.32 | 6.20 (FB) | 2.40 |
| S2tenancy | scatter | 2 | 4 | tp2.dp2.pp1.ep1 | seed5 | 8.71 | 2.88 | 0.33 | 6.20 (FB) | 2.15 |
| S2tenancy | scatter | 4 | 4 | tp2.dp2.pp1.ep1 | seed5 | 9.85 | 3.94 | 0.40 | 11.56 (FB) | 2.93 |
| S2fullmdp | compact | 1 | 16 | tp2.dp2.pp2.ep2 | seed5 | 14.50 | 8.91 | 0.61 | 5.63 (FB) | 0.63 |

### OCS_SCALE (`rd_ocs_scale.csv`, 10 cells)

| series | nodes | deg | placement | jobs | jobnodes | decomp | allreduce | blind | demand | demand/blind | best static | best static / demand |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| fullmdp | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | rhd | 24.81 | 4.86 | 0.20 | 8.44 (FB) | 1.74 |
| fullmdp | 128 | 9 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | ring | 23.52 | 6.05 | 0.26 | 16.22 (FB) | 2.68 |
| fullmdp | 128 | 12 | compact | 1 | 128 | tp4.dp4.pp4.ep2 | rhd | 19.35 | 4.70 | 0.24 | 8.44 (FB) | 1.80 |
| tenancy | 128 | 9 | compact | 1 | 32 | tp2.dp4.pp2.ep2 | rhd | 18.69 | 4.01 | 0.21 | 8.44 (FB) | 2.10 |
| tenancy | 128 | 9 | compact | 2 | 32 | tp2.dp4.pp2.ep2 | rhd | 21.90 | 3.59 | 0.16 | 8.44 (FB) | 2.35 |
| tenancy | 128 | 9 | compact | 4 | 32 | tp2.dp4.pp2.ep2 | rhd | 25.57 | 3.81 | 0.15 | 8.44 (FB) | 2.22 |
| tenancy | 128 | 9 | scatter | 1 | 32 | tp2.dp4.pp2.ep2 | rhd | 15.45 | 2.87 | 0.19 | 16.20 (Dfly) | 5.64 |
| tenancy | 128 | 9 | scatter | 2 | 32 | tp2.dp4.pp2.ep2 | rhd | 17.98 | 2.98 | 0.17 | 19.33 (Jelly) | 6.48 |
| tenancy | 128 | 9 | scatter | 4 | 32 | tp2.dp4.pp2.ep2 | rhd | 22.11 | 3.85 | 0.17 | 27.13 (Jelly) | 7.05 |
| fullmdp | 256 | 12 | compact | 1 | 256 | tp4.dp4.pp4.ep4 | rhd | 26.15 | 6.62 | 0.25 | 8.44 (FB) | 1.28 |

## Summary: does reconfiguring beat the best static family under RoCE?

| experiment | cells with a static comparison | blind dheu wins | demand-aware dheu wins | best static / demand-aware dheu (min-max, mean) |
| --- | --- | --- | --- | --- |
| ROCE_OCS 5d (sweep_ocs) | 1 | 0 | 1 | 1.80-1.80, 1.80 |
| ROCE_OCS 5f (generalise_ocs) | 4 | 2 | 4 | 1.59-3.08, 2.21 |
| LOGICAL_OCS | 7 | 1 | 7 | 1.23-3.71, 1.96 |
| OCS_MT_MDP | 7 | 0 | 6 | 0.68-2.41, 1.56 |
| PFC_HEADROOM, taskgraphs | 28 | 3 | 28 | 1.16-3.72, 1.97 |
| PFC_HEADROOM, MDP | 28 | 0 | 25 | 0.51-2.41, 1.54 |
| DECOMPOSITION | 45 | 1 | 32 | 0.36-3.01, 1.41 |
| PHASED_MDP | 31 | 4 | 24 | 0.61-7.05, 2.02 |
| SEEDS, taskgraphs | 16 | 0 | 16 | 1.14-2.50, 1.65 |
| SEEDS, MDP | 28 | 2 | 24 | 0.63-3.38, 1.58 |
| OCS_SCALE | 10 | 3 | 10 | 1.28-7.05, 3.33 |

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
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp2.dp2.pp2.ep2 rhd | 8.26 | 24.11 | 2.92 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp2.dp2.pp2.ep2 ring | 8.26 | 24.11 | 2.92 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp2.pp2.ep1 rhd | 6.21 | 33.19 | 5.34 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp2.pp2.ep1 ring | 6.06 | 24.30 | 4.01 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp4.pp1.ep1 rhd | 8.81 | 25.53 | 2.90 | 8.44 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp4.dp4.pp1.ep1 ring | 10.62 | 27.73 | 2.61 | 16.22 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp8.dp2.pp1.ep1 rhd | 6.12 | 55.18 | 9.01 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp8.dp2.pp1.ep1 ring | 6.30 | 44.72 | 7.09 | 5.63 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp16.dp1.pp1.ep1 rhd | 3.41 | 66.18 | 19.42 | 1.33 |
| DECOMPOSITION | F16d4 n16 d4 compact j1 tp16.dp1.pp1.ep1 ring | 2.11 | 0.97 | 0.46 | 2.64 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp2.dp2.pp2.ep2 rhd | 3.29 | 12.78 | 3.88 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp2.dp2.pp2.ep2 ring | 3.29 | 12.78 | 3.88 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp2.pp2.ep1 rhd | 2.69 | 22.04 | 8.20 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp2.pp2.ep1 ring | 4.38 | 22.33 | 5.10 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp4.pp1.ep1 rhd | 4.67 | 24.70 | 5.28 | 8.44 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp4.dp4.pp1.ep1 ring | 6.65 | 25.90 | 3.90 | 8.09 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp8.dp2.pp1.ep1 rhd | 3.38 | 45.13 | 13.36 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp8.dp2.pp1.ep1 ring | 3.48 | 45.19 | 12.97 | 5.63 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp16.dp1.pp1.ep1 rhd | 2.89 | 66.09 | 22.84 | 1.33 |
| DECOMPOSITION | F16d6 n16 d6 compact j1 tp16.dp1.pp1.ep1 ring | 0.74 | 0.69 | 0.93 | 1.65 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp3.pp3.ep1 ring | 6.58 | 25.38 | 3.86 | 7.19 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp9.dp3.pp1.ep1 ring | 6.29 | 46.09 | 7.33 | 7.19 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp9.pp1.ep1 ring | 9.34 | 58.95 | 6.31 | 19.20 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp27.dp1.pp1.ep1 ring | 4.48 | 1.65 | 0.37 | 1.63 |
| DECOMPOSITION | F27d6 n27 d6 compact j1 tp3.dp3.pp1.ep3 ring | 7.23 | 34.26 | 4.74 | 7.19 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp2.pp2.ep1 ring | 5.65 | 23.95 | 4.24 | 7.46 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp2.pp2.ep1 auto | 5.65 | 23.95 | 4.24 | 7.46 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp2.dp3.pp2.ep1 ring | 7.95 | 25.87 | 3.26 | 10.80 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp2.dp3.pp2.ep1 auto | 7.95 | 25.87 | 3.26 | 10.80 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp4.pp1.ep1 ring | 8.17 | 27.52 | 3.37 | 19.10 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp3.dp4.pp1.ep1 auto | 6.58 | 27.41 | 4.17 | 14.83 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp4.dp3.pp1.ep1 ring | 7.67 | 22.76 | 2.97 | 10.80 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp4.dp3.pp1.ep1 auto | 7.67 | 33.21 | 4.33 | 10.80 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp12.dp1.pp1.ep1 ring | 2.82 | 1.62 | 0.58 | 2.55 |
| DECOMPOSITION | J12d4 n16 d4 compact j1 tp12.dp1.pp1.ep1 auto | 2.82 | 1.62 | 0.58 | 2.55 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp2.pp2.ep1 ring | 2.39 | 22.61 | 9.45 | 7.20 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp2.pp2.ep1 auto | 2.39 | 22.61 | 9.45 | 7.20 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp2.dp3.pp2.ep1 ring | 5.06 | 22.23 | 4.40 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp2.dp3.pp2.ep1 auto | 5.06 | 22.23 | 4.40 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp4.pp1.ep1 ring | 5.52 | 25.92 | 4.69 | 10.09 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp3.dp4.pp1.ep1 auto | 3.98 | 13.97 | 3.51 | 10.70 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp4.dp3.pp1.ep1 ring | 5.52 | 23.01 | 4.17 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp4.dp3.pp1.ep1 auto | 4.88 | 25.48 | 5.22 | 7.19 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp12.dp1.pp1.ep1 ring | 1.28 | 0.93 | 0.72 | 1.30 |
| DECOMPOSITION | J12d6 n16 d6 compact j1 tp12.dp1.pp1.ep1 auto | 1.28 | 0.93 | 0.72 | 1.30 |
| OCS_SCALE | fullmdp n128 d9 compact j1 tp4.dp4.pp4.ep2 rhd | 4.86 | 34.24 | 7.05 | 8.44 |
| OCS_SCALE | fullmdp n128 d9 compact j1 tp4.dp4.pp4.ep2 ring | 6.05 | 45.23 | 7.48 | 16.22 |
| OCS_SCALE | fullmdp n128 d12 compact j1 tp4.dp4.pp4.ep2 rhd | 4.70 | 35.42 | 7.54 | 8.44 |
| OCS_SCALE | tenancy n128 d9 compact j1 tp2.dp4.pp2.ep2 rhd | 4.01 | 33.33 | 8.30 | 8.44 |
| OCS_SCALE | tenancy n128 d9 compact j2 tp2.dp4.pp2.ep2 rhd | 3.59 | 24.55 | 6.83 | 8.44 |
| OCS_SCALE | tenancy n128 d9 compact j4 tp2.dp4.pp2.ep2 rhd | 3.81 | 34.62 | 9.08 | 8.44 |
| OCS_SCALE | tenancy n128 d9 scatter j1 tp2.dp4.pp2.ep2 rhd | 2.87 | 24.33 | 8.47 | 16.20 |
| OCS_SCALE | tenancy n128 d9 scatter j2 tp2.dp4.pp2.ep2 rhd | 2.98 | 24.52 | 8.23 | 19.33 |
| OCS_SCALE | tenancy n128 d9 scatter j4 tp2.dp4.pp2.ep2 rhd | 3.85 | 23.34 | 6.06 | 27.13 |

DCTCP / RoCE over 54 cells: 0.37-22.84, mean 5.67; RoCE faster in 47.

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
| 2,2,2,2 | 19.39 (6.54 / 19.39) | 5.63 (0.71 / 5.63) | 21.90 (1.54 / 21.90) | 34.47 (5.94 / 34.47) | 11.55 (3.22 / 11.55) | FB |
| 4,2,2,1 | 18.69 (6.31 / 18.69) | 5.63 (1.06 / 5.63) | 21.71 (2.30 / 21.71) | 34.02 (5.87 / 34.02) | 11.39 (4.21 / 11.39) | FB |
| 4,4,1,1 | 21.24 (4.72 / 21.24) | 8.44 (1.06 / 8.44) | 26.79 (2.09 / 26.79) | 38.25 (2.94 / 38.25) | 19.67 (4.28 / 19.67) | FB |
| 8,2,1,1 | 18.70 (7.10 / 18.70) | 5.63 (1.24 / 5.63) | 21.85 (2.70 / 21.85) | 33.20 (4.02 / 33.20) | 12.04 (5.30 / 12.04) | FB |
| 16,1,1,1 | 2.86 (2.86 / NA) | 1.33 (1.33 / NA) | 1.93 (1.93 / NA) | 2.93 (2.93 / NA) | 3.03 (3.03 / NA) | FB |

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
| 2,2,2,2 | 19.39 (6.54 / 19.39) | 5.63 (0.71 / 5.63) | 21.90 (1.54 / 21.90) | 34.47 (5.94 / 34.47) | 11.55 (3.22 / 11.55) | FB |
| 4,2,2,1 | 19.02 (8.22 / 19.02) | 5.63 (2.04 / 5.63) | 21.71 (2.56 / 21.71) | 33.86 (6.36 / 33.86) | 11.99 (4.80 / 11.99) | FB |
| 4,4,1,1 | 25.46 (7.69 / 25.46) | 16.22 (2.04 / 16.22) | 24.86 (2.29 / 24.86) | 32.39 (4.10 / 32.39) | 19.56 (4.22 / 19.56) | FB |
| 8,2,1,1 | 18.88 (8.84 / 18.88) | 5.63 (3.57 / 5.63) | 21.73 (6.52 / 21.73) | 33.78 (7.81 / 33.78) | 12.10 (6.17 / 12.10) | FB |
| 16,1,1,1 | 2.87 (2.87 / NA) | 5.14 (5.14 / NA) | 2.64 (2.64 / NA) | 3.28 (3.28 / NA) | 3.63 (3.63 / NA) | Dfly |

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
| 2,2,2,2 | 17.74 (3.03 / 17.74) | 5.63 (0.71 / 5.63) | 11.05 (1.25 / 11.05) | 34.47 (5.94 / 34.47) | 9.35 (2.49 / 9.35) | FB |
| 4,2,2,1 | 17.41 (4.03 / 17.41) | 5.63 (1.06 / 5.63) | 11.05 (1.61 / 11.05) | 34.02 (5.87 / 34.02) | 9.10 (3.08 / 9.10) | FB |
| 4,4,1,1 | 17.17 (3.21 / 17.17) | 8.44 (1.06 / 8.44) | 14.24 (1.23 / 14.24) | 38.25 (2.94 / 38.25) | 14.04 (3.20 / 14.04) | FB |
| 8,2,1,1 | 17.38 (4.72 / 17.38) | 5.63 (1.24 / 5.63) | 11.06 (2.38 / 11.06) | 33.20 (4.02 / 33.20) | 8.91 (3.70 / 8.91) | FB |
| 16,1,1,1 | 1.89 (1.89 / NA) | 1.33 (1.33 / NA) | 1.59 (1.59 / NA) | 2.93 (2.93 / NA) | 2.63 (2.63 / NA) | FB |

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
| 2,2,2,2 | 17.74 (3.03 / 17.74) | 5.63 (0.71 / 5.63) | 11.05 (1.25 / 11.05) | 34.47 (5.94 / 34.47) | 9.35 (2.49 / 9.35) | FB |
| 4,2,2,1 | 17.38 (3.82 / 17.38) | 5.63 (1.02 / 5.63) | 11.32 (1.66 / 11.32) | 33.86 (6.36 / 33.86) | 9.23 (3.60 / 9.23) | FB |
| 4,4,1,1 | 18.72 (4.61 / 18.72) | 8.09 (1.02 / 8.09) | 16.19 (1.51 / 16.19) | 32.39 (4.10 / 32.39) | 14.04 (3.41 / 14.04) | FB |
| 8,2,1,1 | 17.56 (4.92 / 17.56) | 5.63 (2.38 / 5.63) | 11.19 (2.75 / 11.19) | 33.78 (7.81 / 33.78) | 9.48 (4.85 / 9.48) | FB |
| 16,1,1,1 | 1.65 (1.65 / NA) | 2.57 (2.57 / NA) | 2.57 (2.57 / NA) | 3.28 (3.28 / NA) | 3.36 (3.36 / NA) | Jelly |

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
| 3,3,3,1 | 19.25 (7.08 / 19.25) | 7.19 (0.90 / 7.19) | 14.62 (1.25 / 14.62) | 91.16 (12.94 / 91.16) | 15.62 (3.80 / 15.62) | FB |
| 9,3,1,1 | 17.90 (5.69 / 17.90) | 7.19 (2.42 / 7.19) | 15.82 (4.56 / 15.82) | 91.43 (14.01 / 91.43) | 15.73 (6.15 / 15.73) | FB |
| 3,9,1,1 | 33.01 (5.61 / 33.01) | 19.20 (0.90 / 19.20) | 20.93 (2.25 / 20.93) | 42.97 (3.91 / 42.97) | 22.70 (3.60 / 22.70) | FB |
| 27,1,1,1 | 2.83 (2.83 / NA) | 4.00 (4.00 / NA) | 1.63 (1.63 / NA) | 4.77 (4.77 / NA) | 4.32 (4.32 / NA) | Dfly |
| 3,3,1,3 | 19.31 (7.18 / 19.31) | 7.19 (0.90 / 7.19) | 15.18 (1.72 / 15.18) | 91.24 (13.39 / 91.24) | 16.17 (4.79 / 16.17) | FB |

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
| 3,2,2,1 | 7.46 (4.00 / 7.46) | 18.52 (7.07 / 18.52) | 12.72 (4.82 / 12.72) | 25.20 (6.85 / 25.20) | 12.35 (4.13 / 12.35) | Jelly |
| 2,3,2,1 | 19.37 (4.45 / 19.37) | 10.80 (0.71 / 10.80) | 21.82 (1.20 / 21.82) | 30.39 (3.69 / 30.39) | 15.70 (2.65 / 15.70) | FB |
| 3,4,1,1 | 25.71 (5.09 / 25.71) | 34.60 (7.23 / 34.60) | 19.10 (4.42 / 19.10) | 28.50 (4.53 / 28.50) | 17.87 (3.83 / 17.87) | Dfly |
| 4,3,1,1 | 19.66 (6.16 / 19.66) | 10.80 (2.04 / 10.80) | 21.92 (2.07 / 21.92) | 30.07 (3.75 / 30.07) | 16.78 (4.40 / 16.78) | FB |
| 12,1,1,1 | 2.81 (2.81 / NA) | 5.03 (5.03 / NA) | 2.55 (2.55 / NA) | 3.25 (3.25 / NA) | 3.45 (3.45 / NA) | Dfly |

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
| 3,2,2,1 | 7.46 (4.00 / 7.46) | 18.52 (7.07 / 18.52) | 12.72 (4.82 / 12.72) | 25.20 (6.85 / 25.20) | 12.35 (4.13 / 12.35) | Jelly |
| 2,3,2,1 | 19.37 (4.45 / 19.37) | 10.80 (0.71 / 10.80) | 21.82 (1.20 / 21.82) | 30.39 (3.69 / 30.39) | 15.70 (2.65 / 15.70) | FB |
| 3,4,1,1 | 14.83 (4.52 / 14.83) | 29.31 (10.65 / 29.31) | 17.09 (3.27 / 17.09) | 28.00 (5.18 / 28.00) | 18.35 (3.78 / 18.35) | Jelly |
| 4,3,1,1 | 19.47 (5.06 / 19.47) | 10.80 (1.06 / 10.80) | 21.82 (1.77 / 21.82) | 30.32 (4.57 / 30.32) | 16.06 (4.22 / 16.06) | FB |
| 12,1,1,1 | 2.81 (2.81 / NA) | 5.03 (5.03 / NA) | 2.55 (2.55 / NA) | 3.25 (3.25 / NA) | 3.45 (3.45 / NA) | Dfly |

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
| 3,2,2,1 | 12.04 (2.37 / 12.04) | 11.90 (2.75 / 11.90) | 7.20 (2.30 / 7.20) | 25.20 (6.85 / 25.20) | 10.18 (2.54 / 10.18) | Dfly |
| 2,3,2,1 | 15.30 (2.65 / 15.30) | 7.19 (0.71 / 7.19) | 14.49 (1.25 / 14.49) | 30.39 (3.69 / 30.39) | 11.99 (2.47 / 11.99) | FB |
| 3,4,1,1 | 17.13 (2.95 / 17.13) | 16.20 (1.91 / 16.20) | 10.09 (2.22 / 10.09) | 28.50 (4.53 / 28.50) | 13.46 (2.46 / 13.46) | Dfly |
| 4,3,1,1 | 15.66 (4.10 / 15.66) | 7.19 (1.02 / 7.19) | 14.38 (1.51 / 14.38) | 30.07 (3.75 / 30.07) | 12.02 (3.52 / 12.02) | FB |
| 12,1,1,1 | 1.58 (1.58 / NA) | 2.50 (2.50 / NA) | 1.30 (1.30 / NA) | 3.25 (3.25 / NA) | 2.49 (2.49 / NA) | Dfly |

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
| 3,2,2,1 | 12.04 (2.37 / 12.04) | 11.90 (2.75 / 11.90) | 7.20 (2.30 / 7.20) | 25.20 (6.85 / 25.20) | 10.18 (2.54 / 10.18) | Dfly |
| 2,3,2,1 | 15.30 (2.65 / 15.30) | 7.19 (0.71 / 7.19) | 14.49 (1.25 / 14.49) | 30.39 (3.69 / 30.39) | 11.99 (2.47 / 11.99) | FB |
| 3,4,1,1 | 14.95 (3.26 / 14.95) | 12.34 (2.30 / 12.34) | 10.70 (1.89 / 10.70) | 28.00 (5.18 / 28.00) | 14.19 (2.36 / 14.19) | Dfly |
| 4,3,1,1 | 15.30 (3.18 / 15.30) | 7.19 (1.06 / 7.19) | 14.49 (1.59 / 14.49) | 30.32 (4.57 / 30.32) | 11.86 (3.50 / 11.86) | FB |
| 12,1,1,1 | 1.58 (1.58 / NA) | 2.50 (2.50 / NA) | 1.30 (1.30 / NA) | 3.25 (3.25 / NA) | 2.49 (2.49 / NA) | Dfly |

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
| 16 nodes, RHD (all groups power-of-two) | 0/20 wins, mean 2.78x, worst 3.56x | 20/20 wins, mean 1.00x, worst 1.00x | 0/20 wins, mean 2.53x, worst 4.03x | 0/20 wins, mean 5.04x, worst 6.25x |
| 16 nodes, ring | 3/20 wins, mean 2.56x, worst 3.56x | 16/20 wins, mean 1.16x, worst 1.94x | 1/20 wins, mean 2.41x, worst 4.00x | 1/20 wins, mean 4.59x, worst 6.25x |
| 27 nodes, ring (3-ary) | 0/10 wins, mean 2.21x, worst 2.68x | 8/10 wins, mean 1.29x, worst 2.46x | 2/10 wins, mean 1.67x, worst 2.20x | 0/10 wins, mean 7.49x, worst 12.71x |
| 12-node job on 16 nodes, ring | 3/20 wins, mean 1.66x, worst 2.38x | 8/20 wins, mean 1.57x, worst 2.50x | 9/20 wins, mean 1.58x, worst 3.05x | 1/20 wins, mean 2.96x, worst 4.22x |
| 12-node job on 16 nodes, auto | 5/20 wins, mean 1.57x, worst 2.40x | 8/20 wins, mean 1.53x, worst 2.50x | 7/20 wins, mean 1.59x, worst 3.01x | 1/20 wins, mean 2.98x, worst 4.22x |
| everything except the aligned case (row 1) | 11/70 wins, mean 1.97x, worst 3.56x | 40/70 wins, mean 1.40x, worst 2.50x | 19/70 wins, mean 1.83x, worst 4.00x | 3/70 wins, mean 4.08x, worst 12.71x |
| all cells | 11/90 wins, mean 2.15x, worst 3.56x | 60/90 wins, mean 1.31x, worst 2.50x | 19/90 wins, mean 1.99x, worst 4.03x | 3/90 wins, mean 4.29x, worst 12.71x |

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
| 128/9 | rhd | roce | 33.19 (12.25 / 33.19) | 8.44 (1.06 / 8.44) | 47.74 (5.16 / 47.74) | 141.51 (33.80 / 141.51) | 24.81 (8.06 / 24.81) | FB | 3.93x |
| 128/9 | rhd | dctcp | 30.40 (7.48 / 30.40) | 8.06 (1.01 / 8.06) | 45.98 (1.72 / 45.98) | 133.08 (12.68 / 133.08) | 7.68 (4.64 / 7.68) | FB | 3.77x |
| 128/9 | ring | roce | 33.70 (13.60 / 33.70) | 16.22 (2.04 / 16.22) | 64.95 (5.47 / 64.95) | 167.16 (38.31 / 167.16) | 23.52 (8.39 / 23.52) | FB | 2.08x |
| 128/9 | ring | dctcp | 33.11 (8.47 / 33.11) | 16.13 (2.02 / 16.13) | 64.44 (1.92 / 64.44) | 136.38 (12.79 / 136.38) | 8.26 (7.13 / 8.26) | FB | 2.05x |
| 128/12 | rhd | roce | 33.12 (13.14 / 33.12) | 8.44 (1.06 / 8.44) | 33.49 (2.34 / 33.49) | 141.51 (33.80 / 141.51) | 19.35 (5.81 / 19.35) | FB | 3.92x |
| 128/12 | rhd | dctcp | 33.47 (7.29 / 33.47) | 8.06 (1.01 / 8.06) | 33.62 (1.63 / 33.62) | 133.08 (12.68 / 133.08) | 7.87 (5.04 / 7.87) | FB | 4.15x |
| 256/12 | rhd | roce | 40.64 (15.28 / 40.64) | 8.44 (1.06 / 8.44) | 17.53 (3.15 / 17.53) | 281.94 (127.21 / 281.94) | 26.15 (6.95 / 26.15) | FB | 2.08x |
| 256/12 | rhd | dctcp | 36.34 (8.13 / 36.34) | 8.06 (1.01 / 8.06) | 17.52 (2.24 / 17.52) | 271.41 (31.24 / 271.41) | 10.88 (5.97 / 10.88) | FB | 2.17x |
| 256/12 | ring | roce | 57.16 (23.44 / 57.16) | 8.09 (1.02 / 8.09) | 17.09 (3.62 / 17.09) | 346.84 (68.18 / 346.84) | - | FB | 2.11x |
| 256/12 | ring | dctcp | 51.82 (10.12 / 51.82) | 8.06 (1.01 / 8.06) | 16.90 (2.35 / 16.90) | 319.11 (31.36 / 319.11) | - | FB | 2.10x |

### Tenancy: job 0 with 2 / 4 tenants divided by job 0 alone

| nodes/deg | transport | placement | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 128/9 | roce | compact | 1.03 / 1.11 | 1.00 / 1.00 | 1.00 / 1.00 | 1.01 / 1.02 | 1.17 / 1.37 |
| 128/9 | roce | scatter | 1.14 / 1.60 | 1.19 / 2.73 | 1.43 / 2.19 | 1.87 / 3.67 | 1.16 / 1.43 |
| 128/9 | dctcp | compact | 1.01 / 1.08 | 1.00 / 1.00 | 1.00 / 1.01 | 1.02 / 1.04 | 0.94 / 1.13 |
| 128/9 | dctcp | scatter | 1.07 / 1.39 | 1.19 / 2.79 | 1.49 / 1.90 | 1.73 / 2.99 | 1.12 / 2.05 |
| 256/12 | roce | compact | 1.44 / 1.77 | 1.00 / 1.00 | 1.00 / 1.00 | 0.98 / 1.72 | - |
| 256/12 | roce | scatter | 1.18 / 1.71 | 1.06 / 1.38 | 1.28 / 1.93 | 1.81 / 3.69 | - |
| 256/12 | dctcp | compact | 1.42 / 1.77 | 1.00 / 1.00 | 0.99 / 1.00 | 1.00 / 1.76 | - |
| 256/12 | dctcp | scatter | 1.09 / 1.45 | 1.16 / 1.43 | 1.33 / 1.88 | 1.62 / 3.63 | - |

### Tenancy: job 0 alone, makespan ms

| nodes/deg | transport | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- |
| 128/9 | roce | 32.81 | 8.44 | 56.41 | 49.03 | 18.69 |
| 128/9 | dctcp | 32.41 | 8.06 | 56.33 | 48.88 | 5.04 |
| 256/12 | roce | 24.72 | 8.44 | 45.67 | 142.50 | - |
| 256/12 | dctcp | 23.43 | 8.06 | 48.09 | 130.61 | - |

Non-OK runs: 0

phased
======

### Full MDP: makespan ms by iteration mode

**A: 16 nodes / 4 ports, TP2 DP2 PP2 EP2, roce**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 19.39 | 5.63 | 21.90 | 34.47 | 11.55 | FB < Jelly < Dfly < Mfly | 3.45x |
| ph4c0 | 20.18 | 7.50 | 24.13 | 35.93 | 16.54 | FB < Jelly < Dfly < Mfly | 2.69x |
| ph8c0 | 20.04 | 7.29 | 23.82 | 35.85 | 16.41 | FB < Jelly < Dfly < Mfly | 2.75x |
| ph8c100 | 22.03 | 9.99 | 26.29 | 37.56 | 18.39 | FB < Jelly < Dfly < Mfly | 2.21x |

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
| conc | 33.19 | 8.44 | 47.74 | 141.51 | 24.81 | FB < Jelly < Dfly < Mfly | 3.93x |
| ph8c0 | 34.11 | 11.06 | 52.57 | 145.88 | 31.74 | FB < Jelly < Dfly < Mfly | 3.08x |
| ph8c100 | 36.47 | 14.36 | 55.25 | 145.90 | 34.77 | FB < Jelly < Dfly < Mfly | 2.54x |

**B: 128 nodes / 9 ports, TP4 DP4 PP4 EP2, dctcp**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 53.69 | 8.06 | 55.70 | 152.81 | 34.24 | FB < Jelly < Dfly < Mfly | 6.66x |
| ph8c0 | 87.00 | 10.65 | 171.18 | 273.28 | 312.35 | FB < Jelly < Dfly < Mfly | 8.17x |
| ph8c100 | 65.78 | 13.95 | 162.85 | 238.31 | 200.41 | FB < Jelly < Dfly < Mfly | 4.72x |

**C: 256 nodes / 12 ports, TP4 DP4 PP4 EP4, roce**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 40.64 | 8.44 | 17.53 | 281.94 | NA | FB < Dfly < Jelly < Mfly | 2.08x |
| ph8c0 | 40.15 | 10.42 | 23.01 | 284.71 | NA | FB < Dfly < Jelly < Mfly | 2.21x |
| ph8c100 | 42.70 | 13.72 | 25.78 | 282.45 | NA | FB < Dfly < Jelly < Mfly | 1.88x |

**C: 256 nodes / 12 ports, TP4 DP4 PP4 EP4, dctcp**

| mode | Jelly | FB | Dfly | Mfly | dheu | ranking (static) | FB lead over next static |
| --- | --- | --- | --- | --- | --- | --- | --- |
| conc | 63.75 | 8.06 | 40.03 | 264.71 | NA | FB < Dfly < Jelly < Mfly | 4.97x |
| ph8c0 | 109.64 | 10.03 | 342.69 | 505.43 | NA | FB < Jelly < Dfly < Mfly | 10.93x |
| ph8c100 | 88.79 | 13.33 | 342.69 | 474.82 | NA | FB < Jelly < Dfly < Mfly | 6.66x |

### Tenancy, phased: job 0 with 2 / 4 tenants divided by job 0 alone (concurrent in brackets)

| series | transport | placement | Jelly | FB | Dfly | Mfly | dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| D (128 nodes) | roce | compact | 1.03/1.11 (conc 1.03/1.11) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.01/1.01 (conc 1.01/1.02) | 1.15/1.26 (conc 1.17/1.37) |
| D (128 nodes) | roce | scatter | 1.13/1.57 (conc 1.14/1.60) | 1.22/2.61 (conc 1.19/2.73) | 1.30/1.97 (conc 1.43/2.19) | 1.79/3.33 (conc 1.87/3.67) | 1.12/1.43 (conc 1.16/1.43) |
| D (128 nodes) | dctcp | compact | 1.23/1.87 (conc 1.00/1.06) | 1.00/1.00 (conc 1.00/1.00) | 1.00/0.88 (conc 1.00/1.13) | 1.24/0.93 (conc 0.94/0.96) | 0.68/1.00 (conc 0.74/1.04) |
| D (128 nodes) | dctcp | scatter | 1.27/1.53 (conc 1.00/0.96) | 1.10/2.25 (conc 1.12/1.76) | 1.35/1.81 (conc 1.54/1.72) | 1.76/3.66 (conc 1.45/2.51) | 1.78/2.94 (conc 1.01/0.96) |
| E (16 nodes) | roce | compact | 1.68/1.69 (conc 1.71/1.71) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.00/1.00 (conc 1.00/1.00) | 1.12/1.22 (conc 1.11/1.19) |
| E (16 nodes) | roce | scatter | 1.00/1.00 (conc 1.00/1.00) | 1.05/1.47 (conc 1.00/1.25) | 1.05/1.06 (conc 1.04/1.06) | 1.63/2.94 (conc 1.68/2.97) | 1.15/1.48 (conc 1.17/1.42) |
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
| roce | compact | Jelly | 1.71 | 1.00, 1.00, 2.49, 1.69 |
| roce | compact | FB | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | Dfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | Mfly | 1.00 | 1.00, 1.00, 1.00, 1.00 |
| roce | compact | dheu | 1.19 | 1.36, 1.34, 1.26, 1.32 |
| roce | scatter | Jelly | 1.00 | 1.80, 1.11, 2.15, 1.82 |
| roce | scatter | FB | 1.25 | 1.83, 2.30, 1.00, 1.86 |
| roce | scatter | Dfly | 1.06 | 2.31, 1.61, 1.42, 1.67 |
| roce | scatter | Mfly | 2.97 | 3.05, 3.11, 3.62, 1.61 |
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
| S2fullmdp | roce | 10.42-18.22 | 5.63-5.63 | 21.90-21.90 | 34.47-34.47 | 12.70-14.50 | yes, FB |
| S2fullmdp | dctcp | 8.74-17.86 | 5.37-5.37 | 21.48-21.48 | 33.56-33.56 | 5.25-5.83 | yes, FB |
| S3fullmdp | roce | 29.01-52.09 | 8.44-8.44 | 47.74-47.74 | 141.51-141.51 | - | yes, FB |
| S3fullmdp | dctcp | 28.25-48.03 | 8.06-8.06 | 45.98-45.98 | 133.08-133.08 | - | yes, FB |

### S3. 128 nodes, 4 tenants / alone, per seed

| transport | placement | Jelly | FB | Dfly | Mfly |
| --- | --- | --- | --- | --- | --- |
| roce | compact | 1.78, 1.40, 1.55, 1.48 | 1.00, 1.00, 1.00, 1.00 | 1.00, 1.00, 1.00, 1.00 | 1.02, 1.02, 1.02, 1.02 |
| roce | scatter | 1.63, 1.33, 1.86, 1.94 | 3.21, 2.18, 3.25, 3.31 | 1.65, 1.32, 2.30, 2.16 | 3.00, 2.62, 3.01, 3.24 |
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
| TP4 DP9 PP4 EP3 | roce | 55.27 (17.82 / 55.27) | 106.17 (4.44 / 106.17) | 670.77 (375.75 / 670.77) | 35 / 1.31 |
| TP4 DP9 PP4 EP3 | dctcp | 96.26 (69.85 / 96.26) | 191.57 (1.38 / 191.57) | 731.78 (160.91 / 731.78) | 14 / 0.17 |
| TP8 DP6 PP9 EP1 | roce | 41.89 (12.98 / 41.89) | 99.23 (6.17 / 99.23) | 555.71 (225.80 / 555.71) | 33 / 1.20 |
| TP8 DP6 PP9 EP1 | dctcp | 72.48 (58.55 / 72.48) | 145.63 (33.71 / 145.63) | 508.03 (155.85 / 508.03) | 15 / 0.17 |

### Tenancy, 108-node jobs: job 0 with 2 / 4 tenants divided by job 0 alone (alone ms in brackets)

| transport | placement | Jelly | Dfly | Mfly |
| --- | --- | --- | --- | --- |
| roce | compact | 1.43 / 1.47 (32.7) | 1.00 / 1.00 (79.1) | 1.03 / 1.65 (331.6) |
| roce | scatter | 1.44 / 2.71 (17.0) | 1.25 / 1.74 (28.8) | 2.05 / 4.27 (157.4) |
| dctcp | compact | 1.01 / 1.11 (58.2) | 0.96 / 0.93 (88.6) | 0.95 / 1.66 (249.8) |
| dctcp | scatter | 1.04 / 1.56 (35.1) | 0.88 / 1.10 (49.1) | 1.75 / 3.22 (169.0) |

Non-OK runs: 0

dheuscale
=========

### 256 nodes, one full-MDP job (TP4 DP4 PP4 EP4, RHD), degree 12, ms

| arm | RoCE | DCTCP (exact queue) |
| --- | --- | --- |
| jellyfish | 40.64 | 63.75 |
| flatbutterfly | 8.44 | 8.06 |
| dragonfly | 17.53 | 40.03 |
| megafly | 281.94 | 264.71 |
| dheu, RoCE demand invisible (OCS_SCALE.md) | 26.15 | - |
| **dheu, demand visible (this sweep)** | **6.62** | **34.40** |

### 432 nodes, one full-MDP job, degree 16, ms (static families from OCS_432.md)

| decomposition | transport | Jellyfish | Dragonfly | Megafly | dheu (demand visible) | best static / dheu |
| --- | --- | --- | --- | --- | --- | --- |
| A tp4.dp9.pp4.ep3 | roce | 55.27 | 106.17 | 670.77 | 10.89 | 5.07 |
| A tp4.dp9.pp4.ep3 | dctcp | 96.26 | 191.57 | 731.78 | 71.04 | 1.36 |
| B tp8.dp6.pp9.ep1 | roce | 41.89 | 99.23 | 555.71 | 7.77 | 5.39 |
| B tp8.dp6.pp9.ep1 | dctcp | 72.48 | 145.63 | 508.03 | 55.38 | 1.31 |

### 432 nodes, 1/2/4 jobs of 108 nodes (TP4 DP3 PP3 EP3): job 0's time, ms, and isolation (4 jobs / alone)

| transport | placement | arm | alone | 2 jobs | 4 jobs | 4 jobs / alone |
| --- | --- | --- | --- | --- | --- | --- |
| roce | compact | jellyfish | 32.66 | 46.63 | 48.16 | 1.47 |
| roce | compact | dragonfly | 79.09 | 79.10 | 79.10 | 1.00 |
| roce | compact | megafly | 331.55 | 340.16 | 547.61 | 1.65 |
| roce | compact | dheu | 2.74 | 2.85 | 4.10 | 1.50 |
| roce | scatter | jellyfish | 16.98 | 24.53 | 46.06 | 2.71 |
| roce | scatter | dragonfly | 28.76 | 35.95 | 50.00 | 1.74 |
| roce | scatter | megafly | 157.45 | 323.02 | 673.03 | 4.27 |
| roce | scatter | dheu | 2.68 | 2.93 | 5.40 | 2.01 |
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
| 256 nodes, full MDP, roce | 1979 s | 9.9 s |
| 256 nodes, full MDP, dctcp | 1740 s | 7.3 s |

framework
=========


### A: one 16-node job (TP2 DP2 PP2 EP2), 4 ports; iteration time, ms

| transport | arm | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- | --- |
| roce | jellyfish | 19.39 | 20.04 | 22.03 | 24.09 | 26.96 |
| roce | flatbutterfly | 5.63 | 7.29 | 9.99 | 7.47 | 10.02 |
| roce | dragonfly | 21.90 | 23.82 | 26.29 | 25.66 | 28.31 |
| roce | megafly | 34.47 | 35.85 | 37.56 | 35.92 | 38.26 |
| roce | dheu | 8.26 | 9.75 | 12.18 | 11.59 | 14.29 |
| dctcp | jellyfish | 34.18 | 30.38 | 32.57 | 95.69 | 98.14 |
| dctcp | flatbutterfly | 5.37 | 7.00 | 9.70 | 14.35 | 16.35 |
| dctcp | dragonfly | 31.05 | 47.34 | 50.34 | 64.15 | 67.76 |
| dctcp | megafly | 52.29 | 49.31 | 61.29 | 100.65 | 88.80 |
| dctcp | dheu | 24.11 | 39.25 | 31.28 | 83.54 | 96.30 |

**A: best static family, and the demand-driven fabric against it, per mode**

| transport | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | FB > Jelly > Dfly > Mfly; dheu 0.68x | FB > Jelly > Dfly > Mfly; dheu 0.75x | FB > Jelly > Dfly > Mfly; dheu 0.82x | FB > Jelly > Dfly > Mfly; dheu 0.64x | FB > Jelly > Dfly > Mfly; dheu 0.70x |
| dctcp | FB > Dfly > Jelly > Mfly; dheu 0.22x | FB > Jelly > Dfly > Mfly; dheu 0.18x | FB > Jelly > Dfly > Mfly; dheu 0.31x | FB > Dfly > Jelly > Mfly; dheu 0.17x | FB > Dfly > Mfly > Jelly; dheu 0.17x |

### B: one 128-node job (TP4 DP4 PP4 EP2), 9 ports; iteration time, ms

| transport | arm | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- | --- |
| roce | jellyfish | 33.19 | 34.11 | 36.47 | 43.61 | 46.16 |
| roce | flatbutterfly | 8.44 | 11.06 | 14.36 | 11.15 | 13.70 |
| roce | dragonfly | 47.74 | 52.57 | 55.25 | 63.28 | 67.04 |
| roce | megafly | 141.51 | 145.88 | 145.90 | 198.51 | 198.29 |
| roce | dheu | 4.86 | 18.22 | 20.35 | 27.85 | 32.66 |
| dctcp | jellyfish | 53.69 | 87.00 | 65.78 | 195.02 | 192.71 |
| dctcp | flatbutterfly | 8.06 | 10.65 | 13.95 | 17.39 | 21.01 |
| dctcp | dragonfly | 55.70 | 171.18 | 162.85 | 281.02 | 307.43 |
| dctcp | megafly | 152.81 | 273.28 | 238.31 | 355.19 | 386.87 |
| dctcp | dheu | 34.24 | 312.35 | 200.41 | 391.17 | 463.18 |

**B: best static family, and the demand-driven fabric against it, per mode**

| transport | concurrent | GPipe 8 mb | GPipe 8 mb + compute | 1F1B 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | FB > Jelly > Dfly > Mfly; dheu 1.74x | FB > Jelly > Dfly > Mfly; dheu 0.61x | FB > Jelly > Dfly > Mfly; dheu 0.71x | FB > Jelly > Dfly > Mfly; dheu 0.40x | FB > Jelly > Dfly > Mfly; dheu 0.42x |
| dctcp | FB > Jelly > Dfly > Mfly; dheu 0.24x | FB > Jelly > Dfly > Mfly; dheu 0.03x | FB > Jelly > Dfly > Mfly; dheu 0.07x | FB > Jelly > Dfly > Mfly; dheu 0.04x | FB > Jelly > Dfly > Mfly; dheu 0.05x |

### D: tenancy on 128 nodes - job 0 with three neighbours / alone, per iteration mode

| transport | placement | arm | concurrent | GPipe 8 mb | 1F1B 8 mb + compute |
| --- | --- | --- | --- | --- | --- |
| roce | compact | jellyfish | 1.11 (32.81) | 1.11 (34.01) | 1.08 (39.63) |
| roce | compact | flatbutterfly | 1.00 (8.44) | 1.00 (10.10) | NA (NA) |
| roce | compact | dragonfly | 1.00 (56.41) | 1.00 (57.71) | NA (NA) |
| roce | compact | megafly | 1.02 (49.03) | 1.01 (51.63) | NA (NA) |
| roce | compact | dheu | 0.95 (4.01) | 1.20 (8.72) | NA (NA) |
| roce | scatter | jellyfish | 1.60 (17.00) | 1.57 (18.54) | NA (17.56) |
| roce | scatter | flatbutterfly | 2.73 (25.22) | 2.61 (26.31) | NA (NA) |
| roce | scatter | dragonfly | 2.19 (16.20) | 1.97 (19.40) | NA (NA) |
| roce | scatter | megafly | 3.67 (51.22) | 3.33 (52.74) | NA (NA) |
| roce | scatter | dheu | 1.34 (2.87) | 1.52 (7.63) | NA (NA) |
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
| base | compact | 1.71 / 1.71 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| base | scatter | 1.00 / 1.00 | 1.00 / 1.25 | 1.04 / 1.06 | 1.68 / 2.97 | 1.17 / 1.42 |
| delay | compact | 1.71 / 1.71 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| delay | scatter | 1.00 / 1.00 | 1.00 / 1.25 | 1.04 / 1.06 | 1.66 / 2.96 | 1.17 / 1.42 |
| xoff8 | compact | 1.76 / 1.76 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.12 / 1.27 |
| xoff8 | scatter | 1.00 / 1.00 | 1.00 / 1.20 | 1.06 / 1.08 | 1.63 / 2.77 | 1.15 / 1.45 |
| xoff32 | compact | 1.71 / 1.71 | 1.00 / 1.00 | 1.00 / 1.00 | 1.00 / 1.00 | 1.11 / 1.19 |
| xoff32 | scatter | 1.00 / 1.00 | 1.00 / 1.26 | 1.04 / 1.06 | 1.66 / 3.05 | 1.17 / 1.41 |

Full MDP (16-node TP2 DP2 PP2 EP2 job), makespan ms and the hypercube's lead:

| PFC model | Jelly | FB | Dfly | Mfly | dheu | FB lead over the static families | FB lead over dheu |
| --- | --- | --- | --- | --- | --- | --- | --- |
| base | 19.39 | 5.63 | 21.90 | 34.47 | 11.55 | 3.45-6.12x | 2.05x |
| delay | 19.42 | 5.63 | 21.90 | 34.51 | 11.60 | 3.45-6.13x | 2.06x |
| xoff8 | 20.23 | 5.63 | 23.31 | 35.07 | 12.20 | 3.59-6.23x | 2.17x |
| xoff32 | 19.34 | 5.63 | 21.67 | 33.90 | 11.42 | 3.44-6.02x | 2.03x |
