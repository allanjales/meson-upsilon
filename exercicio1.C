/*
!--------------------------------
!proposito: exercício proposto de root de salvar um histograma para massa invariante do méson upsilon para colisões pp e PbPb
!--------------------------------	
!autor: Allan Jales
!--------------------------------
!v1.5:	Troca da função de sinal para Gaussiana
!--------------------------------
*/

#include "TROOT.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFitResult.h"
#include "TLegend.h"
#include "TLatex.h"
#include "FitFunctions.h" //You should download the file

#include <iostream>

using namespace std;

//Fit function for T(1S), Y(2S), Y(3S)
Double_t Signal(Double_t *x, Double_t *par) {
  return Gaus(x,par) + Gaus(x, &par[3]) + Gaus(x, &par[6]);
}

//Fit function for Background
Double_t Background(Double_t *x, Double_t *par) {
  return Pol3(x, par);
}

//Fit function for T(1S), Y(2S), Y(3S) & background
Double_t FFit(Double_t *x, Double_t *par) {
  return Signal(x,par) + Background(x, &par[9]);
}

//Draws and save invariant mass histogram for PbPb
void invariantmassPbPb(){
	TFile *file0 = TFile::Open("upsilonTree_2p76TeV_PbPb_data.root");	//Opens the file
	TTree *upsilonTree = (TTree*)file0->Get("UpsilonTree");			//Opens TTree from file
	
	//Creates variables
	float invariantMass;
	float muPlusPt;
	float muMinusPt;
	int QQsign;

	int i;	//Itineration variable

	//Assign variables
	upsilonTree->SetBranchAddress("invariantMass",	&invariantMass);
	upsilonTree->SetBranchAddress("muPlusPt",	&muPlusPt);
	upsilonTree->SetBranchAddress("muMinusPt",	&muMinusPt);
	upsilonTree->SetBranchAddress("QQsign",		&QQsign);

	TH1F *h1 = new TH1F("InvariantMassPbPb","Invariant Mass for PbPb;Mass_{#mu^{+}#mu^{-}} (GeV);Counts",100,7,14);	//Creates histogram

	for (i = 0; i < upsilonTree->GetEntries(); i++){	//Loop between the components
	
		upsilonTree->GetEntry(i);			//Gets the entry from TTree

		if (muPlusPt <= 4 && muMinusPt <= 4) continue;	//If the pair hasn't Pt > 4 GeV, skip

		if (QQsign != 0) continue;			//If hasn't opposite charge, skip

		h1->Fill(invariantMass);			//Fill the histogram then
	}

	TCanvas *c1 = new TCanvas("c1","Invariant Mass for PbPb", 600, 600);	//Creates Canvas for drawing

	//Set margin for canvas
	c1->SetTopMargin(0.05);
	c1->SetRightMargin(0.05);
	c1->SetBottomMargin(0.12);
	c1->SetLeftMargin(0.13);

	h1->SetMarkerStyle(20);		//Set markers style
	h1->SetMarkerColor(kRed);	//Set markers colors
	h1->SetLineColor(kRed);		//Set lines colors (for errorbars)



	//Fit Function
	TF1 *f = new TF1("f",FFit,7.,14.,13);
	f->SetParName(0,	"Gaus(1S) a (altura)");
	f->SetParName(1,	"Gaus(1S) b (posicao)");
	f->SetParName(2,	"Gaus(1S) c (largura)");
	f->SetParName(3,	"Gaus(2S) a (altura)");
	f->SetParName(4,	"Gaus(2S) b (posicao)");
	f->SetParName(5,	"Gaus(2S) c (largura)");
	f->SetParName(6,	"Gaus(3S) a (altura)");
	f->SetParName(7,	"Gaus(3S) b (posicao)");
	f->SetParName(8,	"Gaus(3S) c (largura)");
	f->SetParName(9,	"Pol3(Bg) d");
	f->SetParName(10,	"Pol3(Bg) c");
	f->SetParName(11,	"Pol3(Bg) b");
	f->SetParName(12,	"Pol3(Bg) a");
	f->SetNpx(1000);	//Resolution of fit function

	//Values Y(1S)
	f->SetParameter(0, 2.0);
	f->FixParameter(1, 9.4603); //PDG value
	f->SetParameter(2, 1300.0);

	//Values Y(2S)
	f->SetParameter(3, 2.0);
	f->FixParameter(4, 10.02326); //PDG value
	f->SetParameter(5, 100.0);

	//Values Y(3S)
	f->SetParameter(6, 2.0);
	f->FixParameter(7, 10.3552); //PDG value
	f->SetParameter(8, 50.0);

	//Values Background
	f->SetParameter(9, 1);
	f->SetParameter(10, 1);
	f->SetParameter(11, 1);	
	f->SetParameter(12, 1);

	f->SetParLimits(9, -10000, 1000);
	f->SetParLimits(10, -10000, 1000);
	f->SetParLimits(11, -10000, 1000);
	f->SetParLimits(12, -10000, 1000);


	f->SetLineColor(kBlue); 	//Fit Color

	//Fit the function
	TFitResultPtr fitr = h1->Fit(f,"RNS","",7.,14.);


	
	//CrystallBall for Background	
	TF1 *fb = new TF1("fb",FFit,7,14,5);
	fb->SetParNames("CB Alpha","CB n","CB Mean","CB Sigma","CB Yield");

	//Get parameters from fit function and set on background function
   	Double_t par[13];
	f->GetParameters(par);
	fb->SetParameters(&par[9]);

	fb->SetLineColor(kBlue); 	//Fit Color
	fb->SetLineStyle(kDashed);	//Fit Style
	


	//Not show frame with mean, std dev
	gStyle->SetOptStat(0);

	//Draws
	h1->Draw("ep");
	f->Draw("same");
	//fb->Draw("same");

	//Draws information
	TLatex *tx = new TLatex();
	tx->SetTextSize(0.04);
	tx->SetTextAlign(12);
	tx->SetTextFont(42);
	tx->SetNDC(kTRUE);

	tx->DrawLatex(0.6,0.6,Form("P^{#mu^{+}}_{T} > 3.5 GeV"));
	tx->DrawLatex(0.6,0.52,Form("P^{#mu^{-}}_{T} > 4 GeV"));
	tx->DrawLatex(0.6,0.44,Form("#chi^{2}/ndf = %g/%d",fitr->Chi2(),fitr->Ndf()));
	
	//Mostra legenda
	TLegend *l = new TLegend(0.74,0.78,0.90,0.90);
	l->SetTextSize(0.04);
	l->AddEntry(h1,"#Upsilon#rightarrow#mu#mu","lp");
	l->AddEntry(f,"Fit","l");
	l->Draw();

	c1->SaveAs("InvariantMassPbPb.png");	//Saves as image
}

//Draws and save invariant mass histogram for pp
void invariantmasspp(){
	TFile *file0 = TFile::Open("upsilonTree_2p76TeV_pp_data.root");	//Opens the file
	TTree *upsilonTree = (TTree*)file0->Get("UpsilonTree");		//Opens TTree from file
	
	//Creates variables
	float invariantMass;
	float muPlusPt;
	float muMinusPt;
	int QQsign;

	int i;	//Itineration variable

	//Assign variables
	upsilonTree->SetBranchAddress("invariantMass",	&invariantMass);
	upsilonTree->SetBranchAddress("muPlusPt",	&muPlusPt);
	upsilonTree->SetBranchAddress("muMinusPt",	&muMinusPt);
	upsilonTree->SetBranchAddress("QQsign",		&QQsign);

	TH1F *h1 = new TH1F("InvariantMasspp","Massa Invariante para pp;Massa_{#mu^{+}#mu^{-}} (GeV/c^{2});Eventos / (0,05 GeV/c^{2})",130,7.5,14.);	//Creates histogram

	for (i = 0; i < upsilonTree->GetEntries(); i++){	//Loop between the components
	
		upsilonTree->GetEntry(i);			//Gets the entry from TTree

		if (muPlusPt <= 4 && muMinusPt <= 4) continue;	//If the pair hasn't Pt > 4 GeV, skip

		if (QQsign != 0) continue;			//If hasn't opposite charge, skip

		h1->Fill(invariantMass);			//Fill the histogram then
	}

	TCanvas *c1 = new TCanvas("c1","Invariant Mass for pp", 600, 600);	//Creates Canvas for drawing

	//Set margin for canvas
	//c1->SetTopMargin(0.05);
	c1->SetTopMargin(0.07);
	c1->SetRightMargin(0.05);
	//c1->SetBottomMargin(0.12);
	c1->SetBottomMargin(0.10);
	c1->SetLeftMargin(0.13);

	//h1->SetBit(TH1::kNoTitle);	//Not show histogram title
	h1->SetMarkerStyle(20);		//Set markers style
	h1->SetMarkerColor(kBlack);	//Set markers colors
	h1->SetLineColor(kBlack);		//Set lines colors (for errorbars)



	//Fit Function
	TF1 *f = new TF1("f",FFit,7.,14.,13);
	f->SetParName(0,	"Gaus(1S) a (altura)");
	f->SetParName(1,	"Gaus(1S) b (posicao)");
	f->SetParName(2,	"Gaus(1S) c (largura)");
	f->SetParName(3,	"Gaus(2S) a (altura)");
	f->SetParName(4,	"Gaus(2S) b (posicao)");
	f->SetParName(5,	"Gaus(2S) c (largura)");
	f->SetParName(6,	"Gaus(3S) a (altura)");
	f->SetParName(7,	"Gaus(3S) b (posicao)");
	f->SetParName(8,	"Gaus(3S) c (largura)");
	f->SetParName(9,	"Pol3(Bg) d");
	f->SetParName(10,	"Pol3(Bg) c");
	f->SetParName(11,	"Pol3(Bg) b");
	f->SetParName(12,	"Pol3(Bg) a");
	f->SetNpx(1000);	//Resolution of fit function

	//Values Y(1S)
	f->SetParameter(0, 510.0);
	f->FixParameter(0, 300.0);
	f->FixParameter(1, 9.4603); //PDG value
	f->SetParameter(2, 0.12);
	f->FixParameter(2, 0.097);

	//Values Y(2S)
	f->SetParameter(3, 112.0);
	f->FixParameter(4, 10.02326); //PDG value
	f->SetParameter(5, 0.102);

	//Values Y(3S)
	f->SetParameter(6, 45.3);
	f->FixParameter(7, 10.3552); //PDG value
	f->SetParameter(8, 0.09);

	//Values Background
	f->SetParameter(9, -1205.0);
	f->SetParameter(10, 385.7);
	f->SetParameter(11, -36.3);
	f->SetParameter(12, 1.1);

	f->SetLineColor(kBlue); //Fit Color

	//Fit the function
	TFitResultPtr fitr = h1->Fit(f,"RNS","",7.,14.);

	//Draws histogram & fit function
	h1->Draw("ep");
	f->Draw("same");



	//Get parameters from fit function and put it in par variable
   	Double_t par[13];
	f->GetParameters(par);


	//Signal Fitting
	TF1 *fs = new TF1("fs",Signal,7.,14.,15);
	fs->SetNpx(1000);				//Resolution of background fit function
	fs->SetParameters(par);			//Get only background part
	fs->SetLineColor(kMagenta); 	//Fit Color
	fs->SetLineStyle(kSolid);		//Fit Style
	fs->Draw("same");				//Draws

	//Background Fitting
	TF1 *fb = new TF1("fb",Background,7.,14.,4);
	fb->SetNpx(1000);				//Resolution of background fit function
	fb->SetParameters(&par[9]);		//Get only background part
	fb->SetLineColor(kBlue); 		//Fit Color
	fb->SetLineStyle(kDashed);		//Fit Style
	fb->Draw("same");				//Draws
	

	//Y(1S) Fitting	(for integration purpose)
	TF1 *f1 = new TF1("f1",Gaus,7.,14.,3);
	f1->SetParameters(par);			//Get only Y(1S) part
	
	//Y(1S) Fitting
	TF1 *f2 = new TF1("f2",Gaus,7.,14.,3);
	f2->SetParameters(&par[3]);		//Get only Y(2S) part
	
	//Y(1S) Fitting
	TF1 *f3 = new TF1("f3",Gaus,7.,14.,3);
	f3->SetParameters(&par[6]);		//Get only Y(3S) part
	


	//Not show frame with mean, std dev
	gStyle->SetOptStat(0);

	//Draws information
	TLatex *tx = new TLatex();
	tx->SetTextSize(0.04);
	tx->SetTextAlign(12);
	tx->SetTextFont(42);
	tx->SetNDC(kTRUE);

	tx->DrawLatex(0.6,0.6,Form("P^{#mu^{+}}_{T} > 3.5 GeV/c"));
	tx->DrawLatex(0.6,0.52,Form("P^{#mu^{-}}_{T} > 4 GeV/c"));
	tx->DrawLatex(0.6,0.44,Form("#chi^{2}/ndf = %g/%d",fitr->Chi2(),fitr->Ndf()));
	
	//Mostra legenda
	//TLegend *l = new TLegend(0.72,0.84,0.90,0.90);
	//TLegend *l = new TLegend(0.72,0.78,0.90,0.90);
	TLegend *l = new TLegend(0.72,0.68,0.90,0.90);
	l->SetTextSize(0.04);
	l->AddEntry(h1,"#Upsilon#rightarrow#mu^{+}#mu^{-}","lp");
	l->AddEntry(f,"Ajuste","l");
	l->AddEntry(fb,"Fundo","l");
	l->AddEntry(fs,"Sinal","l");
	l->Draw();

	c1->SaveAs("InvariantMasspp.png");	//Saves as image

	cout << endl;
	cout << "Chi2/ndf = " << f->GetChisquare()/f->GetNDF() << endl;
	cout << endl;

	//Integral da área da função sinal Intergral(intervalo)*quantidade de bins por unidade
	cout << "Área do Y(1S) = " << f1->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << "Área do Y(2S) = " << f2->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << "Área do Y(3S) = " << f3->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << "Área de sinal = " << fs->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << "Área de fundo = " << fb->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << endl;
	cout << "Área total    = " << f ->Integral(h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax()) * (1/h1->GetBinWidth(0)) << endl;
	cout << endl;	

}

//Calls functions to draw and save invariant mass histogram
void exercicio1() {
	//invariantmassPbPb();	//Draws and save invariant mass histogram for PbPb collision
	invariantmasspp();	//Draws and save invariant mass histogram for pp collision
}
