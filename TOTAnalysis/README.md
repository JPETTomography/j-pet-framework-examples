# Template for custom analysis

## Aim
This is an example of  performing study to develop a relationship between Energy deposition by incident photon in interaction with plastic scintillator and the measured TOT value. 
For the details please refer the article : [S. Sharma et al., https://arxiv.org/abs/1911.12059.]

Some pointers in using  the algorithem: 

1.  Events with only 3 hits are selected.
2. Analysis is divided in two parts: 
	a). Tagging photons of two different energies 511 keV and 1275 keV . Eventwise calculating the scattering of respective photons (primary) and calculate the  
	     energy deposition. 
	b). For the associated of scatterd photon to its primary interaction, scattering test is used. For this purpose, elliptical cuts ( equations ) are provided in the  
	     code, one can modify the shape of ellipse  by changing the parameters. 
	c). Once we have energy deposition for a hit, we have one-to-one correspondence of energy loss in a hit and the corresponding measured TOT value.
3). In selecting the 511 keV photons or 1275 keV photons we are not using sharp cuts on TOT spectra, only the higher limits to cover the full range of energy depositions.
4). Apart from higher ranges cut on TOT, the selection of photons are also based on the angular correlations of the 3 hits ( sum of two smallest vs their difference azimuth angles between the hits).
5). Two methods are in code : (i) Based on multiple of any threshold values ( e.g., run4 - 80,160,240,320 like in Run 4) : calculateSumOfTOTs(hit)
(ii) Irregular thresholds values - one has to normalize  ( Run 7, 9, 11): calculateTOTAdjusted(hit)

## Author 
Please report any bugs and suggestion of corrections to : [sushil.sharma@uj.edu.pl] or any core member from the framework developers team. 
