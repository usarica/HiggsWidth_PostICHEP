// Configuration options
{
	"inputDirectory":"/home/ianderso/Work/HighMass/AnomalousCouplings/HiggsWidth_PostICHEP/LHC_7TeV/Analysis/Templates/Combine/",
	"outputFile":"/home/ianderso/Work/HighMass/AnomalousCouplings/HiggsWidth_PostICHEP/LHC_7TeV/Analysis/Templates/Combine/2mu2e/HtoZZ4l_MCFM_125p6_SmoothTemplates__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root",
	// template definitions
	"templates":[
		// T_1 
		{
			"name":"T_2D_1",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_1_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":50},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,900,1000,1100,1200,1300,1400,1500,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}
			]
		},
		// T_2
		{
			"name":"T_2D_2",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_2_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":100},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,860,920,980,1040,1100,1160,1220,1280,1340,1400,1460,1520,1580,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}

			]
		},
		// T_4
		{
			"name":"T_2D_4",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_4_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":100},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,860,920,980,1040,1120,1200,1320,1440,1560,1580,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}

			]
		},
		// T_qqZZ
		{
			"name":"T_2D_qqZZ",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_qqZZ_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":75},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,900,1000,1100,1200,1300,1420,1560,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}

			]
		},
		// T_ZX
		{
			"name":"T_2D_ZX",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_ZX_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":200},
				{"type":"reweight", "axes":[1],
				       	"rebinning":[
						[220,240,280,320,360,400,460,520,580,680,820,1000,1280,1600],
						[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.23333333333333333,0.3,0.3333333333333333,0.36666666666666667,0.4,0.4666666666666667,0.5333333333333333,0.6,0.6666666666666667,0.7333333333333333,0.8,0.8666666666666667,0.9333333333333333,0.9666666666666667,1.0]
						]
				},
				{"type":"floor"}

			]
		},
		// T_ZX_merged
		{
			"name":"T_2D_ZX_merged",
			"files":[
				"./4mu/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root",
				"./4e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root",
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_ZX_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":200},
				{"type":"reweight", "axes":[0,1],
				       	"rebinning":[
						[220,240,280,320,360,400,460,520,580,680,820,1000,1280,1600],
						[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.23333333333333333,0.3,0.3333333333333333,0.36666666666666667,0.4,0.4666666666666667,0.5333333333333333,0.6,0.6666666666666667,0.7333333333333333,0.8,0.8666666666666667,0.9333333333333333,0.9666666666666667,1.0]
						]
				},
				{"type":"floor"},
				{"type":"rescale","factor":0.506761}

			]
		},
		// T_VBF_1 
		{
			"name":"T_2D_VBF_1",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_VBF_1_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":200},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,860,920,980,1040,1120,1200,1320,1440,1560,1580,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}

			]
		},
		// T_VBF_2
		{
			"name":"T_2D_VBF_2",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_VBF_2_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":200},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,860,920,980,1040,1120,1200,1320,1440,1560,1580,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}

			]
		},
		// T_VBF_4
		{
			"name":"T_2D_VBF_4",
			"files":[
				"./2mu2e/HtoZZ4l_MCFM_125p6_ModifiedTemplateswithTreesForCombine__GenLevelVBF_wResolution_D_Gamma_gg_r10_SysDown_ggQCD.root"
				],
			"tree":"T_2D_VBF_4_Tree",
			"variables":["ZZMass","D_Gamma_gg_r10"],
			"weight":"templateWeight",
			"conserveSumOfWeights":true,
			"selection":"ZZMass>=220 && ZZMass<=1600",
			"assertion":"1",
			"binning":{
				"type":"fixed",
				"bins":[69,220.,1600.,30,0.,1.]
			},
			"postprocessing":[
				{"type":"smooth", "kernel":"adaptive", "entriesperbin":200},
				//{"type":"reweight", "axes":[0,1],
				//       	"rebinning":[
				//		[220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600,640,680,740,800,860,920,980,1040,1120,1200,1320,1440,1560,1580,1600],
				//		[0.0,0.03333333333333333,0.06666666666666667,0.1,0.13333333333333333,0.16666666666666667,0.2,0.23333333333333333,0.26666666666666666,0.3,0.3333333333333333,0.36666666666666667,0.4,0.43333333333333333,0.4666666666666667,0.5,0.5333333333333333,0.5666666666666667,0.6,0.6333333333333333,0.6666666666666667,0.7,0.7333333333333333,0.7666666666666667,0.8,0.8333333333333333,0.8666666666666667,0.9,0.9333333333333333,0.9666666666666667,1.0]	
				//		]
				//},
				{"type":"floor"}
			]
		}
	]
}
