/*
!--------------------------------
!proposito: ajustar funções sobre histogramas no auxilio do tag and probe
!--------------------------------	
!autor: Allan Jales
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

#include <iostream>

using namespace std;



//-------------------------------------
// General functions
//-------------------------------------

//Gaussian function
Double_t Gaus(Double_t *x, Double_t *par)
{
	//par[0] = height
	//par[1] = position
	//par[2] = sigma
	Double_t gaus = par[0]*TMath::Gaus(x[0],par[1],par[2],1);
	return gaus;
}

//Polynomial function
Double_t Pol1(Double_t *x, Double_t *par)
{
	//par[0] = b
	//par[1] = a
	Double_t pol = par[0] + par[1]*x[0];
	return pol;
}

//Exponential function
Double_t Exp(Double_t *x, Double_t *par)
{
	//par[0] = height
	//par[1] = width
	Double_t exp = par[0] * TMath::Exp(par[1]*x[0]);
	return exp;
}

//crystall ball function
Double_t CrystalBall(Double_t *x,Double_t *par)
{
	//par[0] = alpha
	//par[1] = n
	//par[2] = mean
	//par[3] = sigma
	//par[4] = Yield
	Double_t t = (x[0]-par[2])/par[3];
	if (par[0] < 0) t = -t;
	Double_t absAlpha = fabs((Double_t)par[0]);
	if (t >= -absAlpha)
	{
		return par[4]*exp(-0.5*t*t);
	}
	else
	{
		Double_t a =  TMath::Power(par[1]/absAlpha,par[1])*exp(-0.5*absAlpha*absAlpha);
		Double_t b = par[1]/absAlpha - absAlpha;
		return par[4]*(a/TMath::Power(b - t, par[1]));
	}
}



//-------------------------------------
// Fit functions
//-------------------------------------

//Fit function for signal for Invariant Mass Probe
Double_t Signal_InvariantMassProbe(Double_t *x, Double_t *par) {
	return Gaus(x,par) + CrystalBall(x, &par[3]);
}

//Fit function for background for Invariant Mass Probe
Double_t Background_InvariantMassProbe(Double_t *x, Double_t *par) {
	return Exp(x,par) + Exp(x, &par[2]);
}

//Fit function for signal & background for Invariant Mass Probe
Double_t FFit_InvariantMassProbe(Double_t *x, Double_t *par) {
	return Signal_InvariantMassProbe(x,par) + Background_InvariantMassProbe(x, &par[8]);
}



//-------------------------------------
// Main functions
//-------------------------------------

//Draws and save invariant mass histogram for pp
void invariantMassProbe()
{
	TFile *file0 = TFile::Open("data_histoall.root");				//Opens the file
	TTree *Tree = (TTree*)file0->Get("demo/AnalysisTree");			//Opens TTree from file
	
	//Creates variables
	Double_t 	ProbeMuon_Pt;
	Double_t 	ProbeMuon_Eta;
	Double_t 	InvariantMass;

	int i;	//Itineration variable

	//Variables for side band subtraction
	int count_sigregion = 0;	//Inside sigma
	int count_sideband = 0;		//Outside sigma

	//Jpsi constants (PDG values)
	const double M_JPSI = 3.097;
	const double W_JPSI = 0.010;

	//Assign variables
	Tree->SetBranchAddress("ProbeMuon_Pt",		&ProbeMuon_Pt);
	Tree->SetBranchAddress("ProbeMuon_Eta",		&ProbeMuon_Eta);
	Tree->SetBranchAddress("InvariantMass",		&InvariantMass);

	//Create histograms
	TH1D *hMassAll = new TH1D("InvariantMassProbe","All Invariant Mass (Probe);Mass (GeV/c^{2});Events",240,2.8,3.4);
	TH1D *hMassSig = new TH1D("InvariantMassProbe","Invariant Mass of Signal (Probe);Mass (GeV/c^{2});Events",240,2.8,3.4);
	
	//Transversal Momentum histogram
	TH1D *hPtBack = new TH1D("TransversalMomentumProbeBackground","Transversal Momentum (Probe);x;Events",240,3.,30.);
	TH1D *hPtSigBack = new TH1D("TransversalMomentumProbeAll","Transversal Momentum (Probe);x;Events",240,3.,30.);

	//Pseudorapidity histogram
	TH1D *hEtaBack = new TH1D("TransversalMomentumProbe","Pseudorapidity (Probe);x;Events",240,-1.6,1.6);
	TH1D *hEtaSigBack = new TH1D("TransversalMomentumProbe","Pseudorapidity (Probe);x;Events",240,-1.6,1.6);

	//Reset title for Y axis wth the right bin width
	hMassAll->GetYaxis()->SetTitle(Form("Events / (%1.4f GeV/c^{2})", hMassAll->GetBinWidth(0)));


	//Loop between the components
	for (i = 0; i < Tree->GetEntries(); i++)
	{
		//Gets the entry from TTree
		Tree->GetEntry(i);

		//Fill the histogram then
		hMassAll->Fill(InvariantMass);

		//if is inside signal region
		if (fabs(InvariantMass - M_JPSI) < W_JPSI*3.0)
		{
			hPtSigBack->Fill(ProbeMuon_Pt);
			hEtaSigBack->Fill(ProbeMuon_Eta);
			count_sigregion++;
		}

		//If is inside sideband region
		if (fabs(InvariantMass - M_JPSI) > W_JPSI*3.5 && fabs(InvariantMass - M_JPSI) < W_JPSI*6.5)
		{
			hPtBack->Fill(ProbeMuon_Pt);	//Add to Pt histogram
			hEtaBack->Fill(ProbeMuon_Eta);	//Add to Eta hitogram
			count_sideband++;
		}
	}

	///Subtraction for Pt
	TH1D* hPtSig = (TH1D*) hPtSigBack->Clone("hPtSig");		//Clone histogram
	hPtSig->Add(hPtBack,-1);								//Subtract histogram

	///Subtraction for Eta
	TH1D* hEtaSig = (TH1D*) hEtaSigBack->Clone("hEtaSig");	//Clone histogram
	hEtaSig->Add(hEtaBack,-1);								//Subtract histogram

	//Number of signal and uncertain
	double S = count_sigregion - count_sideband;
	double dS = sqrt(count_sigregion+count_sideband);



	//-------------------------------------
	// Canvas Invariant Mass
	//-------------------------------------

	//Create canvas
	TCanvas *c1 = new TCanvas("c1","Invariant Mass", 600, 600);			//Creates Canvas for drawing

	//Set margin for canvas
	c1->SetTopMargin(0.07);
	c1->SetRightMargin(0.05);
	c1->SetBottomMargin(0.11);
	c1->SetLeftMargin(0.15);

	//Set title size for axis and other stuffs for histogram style
	hMassAll->GetYaxis()->SetTitleSize(0.04);
	hMassAll->GetXaxis()->SetTitleSize(0.05);
	hMassAll->GetXaxis()->CenterTitle(true);
	//hMassAll->SetBit(TH1::kNoTitle);			//Not show histogram title
	hMassAll->SetMarkerStyle(20);				//Set markers style
	hMassAll->SetMarkerColor(kBlack);			//Set markers colors
	hMassAll->SetLineColor(kBlack);				//Set lines colors (for errorbars)



	//Fit Function
	TF1 *f = new TF1("f",FFit_InvariantMassProbe,hMassAll->GetXaxis()->GetXmin(),hMassAll->GetXaxis()->GetXmax(),12);
	f->SetParName(0,	"Gaus(Sg) Height");
	f->SetParName(1,	"Gaus(Sg) Position");
	f->SetParName(2,	"Gaus(Sg) Sigma");
	f->SetParName(3,	"CB(Sg) Alpha");
	f->SetParName(4,	"CB(Sg) N");
	f->SetParName(5,	"CB(Sg) Mean");
	f->SetParName(6,	"CB(Sg) Sigma");
	f->SetParName(7,	"CB(Sg) Yield");
	f->SetParName(8,	"Exp1(Bg) Lambda");
	f->SetParName(9,	"Exp1(Bg) a");
	f->SetParName(10,	"Exp2(Bg) Lambda");
	f->SetParName(11,	"Exp2(Bg) a");
	f->SetNpx(1000);	//Resolution of fit function
	
	//Values Signal
	//f->SetParameter(0,	2000.0);
	f->SetParameter(1,	3.1);
	f->SetParameter(2,	0.021);
	f->SetParameter(3,	2.1);
	f->SetParameter(4,	0.40);
	f->SetParameter(5,	3.094);
	f->SetParameter(6,	0.02);
	//f->SetParameter(7,	0);

	//Values Background
	f->SetParameter(8,	-1.7);
	f->SetParameter(9,	2.0);
	//f->SetParameter(10, -1.7);
	//f->SetParameter(11,	2.0);

	//Fit Color
	f->SetLineColor(kBlue);

	//Fit the function
	TFitResultPtr fitr = hMassAll->Fit(f,"RNS","",hMassAll->GetXaxis()->GetXmin(),hMassAll->GetXaxis()->GetXmax());


	//Get parameters from fit function and put it in par variable
   	Double_t par[12];
	f->GetParameters(par);

	//Signal Fitting
	TF1 *fs = new TF1("fs",Signal_InvariantMassProbe,hMassAll->GetXaxis()->GetXmin(),hMassAll->GetXaxis()->GetXmax(),8);
	fs->SetNpx(1000);				//Resolution of background fit function
	fs->SetParameters(par);			//Get only background part
	fs->SetLineColor(kMagenta); 	//Fit Color
	fs->SetLineStyle(kSolid);		//Fit Style

	//Background Fitting
	TF1 *fb = new TF1("fb",Background_InvariantMassProbe,hMassAll->GetXaxis()->GetXmin(),hMassAll->GetXaxis()->GetXmax(),4);
	fb->SetNpx(1000);				//Resolution of background fit function
	fb->SetParameters(&par[8]);		//Get only background part
	fb->SetLineColor(kBlue); 		//Fit Color
	fb->SetLineStyle(kDashed);		//Fit Style

	//Draws histogram & fit function
	hMassAll->Draw("ep");
	fs->Draw("same");
	fb->Draw("same");
	f->Draw("same");


	//Draws information
	TLatex *tx = new TLatex();
	tx->SetTextSize(0.04);
	tx->SetTextAlign(12);
	tx->SetTextFont(42);
	tx->SetNDC(kTRUE);

	//Show chi-squared test
	tx->DrawLatex(0.6,0.60,Form("#chi^{2}/ndf = %g/%d",fitr->Chi2(),fitr->Ndf()));

	//Show number of particles
	tx->DrawLatex(0.6,0.48,Form("%.1f #pm %.1f J/#psi", S, dS));


	//Add legend
	TLegend *l = new TLegend(0.65,0.75,0.92,0.90);
	l->SetTextSize(0.04);
	l->AddEntry(hMassAll,	"J/#psi","lp");
	l->AddEntry(f,			"Fitting","l");
	l->AddEntry(fs,			"Signal","l");
	l->AddEntry(fb,			"Background","l");
	l->Draw();


	//Not show frame with mean, std dev
	gStyle->SetOptStat(0);



	//-------------------------------------
	// Canvas Transversal Momentum
	//-------------------------------------

	//Create canvas
	TCanvas *c2 = new TCanvas("c2","Transversal Momentum", 1200, 600);

	//Divide canvas
	c2->Divide(2,1);

	//Select canvas part
	c2->cd(1);

	//Draws histogram
	hPtSigBack->Draw();

	//Background
	hPtBack->SetLineColor(kBlue); 		//Fit Color
	hPtBack->SetLineStyle(kDashed);		//Fit Style
	hPtBack->Draw("same");

	//Signal
	hPtSig->SetLineColor(kMagenta); 	//Fit Color
	hPtSig->Draw("same");


	//Select canvas part
	c2->cd(2);

	//Same range as comparision
	//hPtSig->GetYaxis()->SetRangeUser(0,hPtSigBack->GetYaxis()->GetYmax());
	hPtSig->SetMinimum(0);
	hPtSig->SetTitle("Transversal Momentum of Signal (Probe)");

	//Signal
	hPtSig->SetLineColor(kMagenta); 		//Fit Color
	hPtSig->Draw("same");


	//-------------------------------------
	// Canvas Transversal Momentum
	//-------------------------------------

	//Create canvas
	TCanvas *c3 = new TCanvas("c3","Pseudorapidity", 1200, 600);

	//Divide canvas
	c3->Divide(2,1);

	//Select canvas part
	c3->cd(1);

	//Same range as comparision
	//hPtSig->GetYaxis()->SetRangeUser(0,hPtSigBack->GetYaxis()->GetYmax());
	hEtaSig->SetMinimum(0);
	hEtaSig->SetTitle("Pseudorapidity of Signal (Probe)");

	//Draws histogram
	hEtaSigBack->Draw();

	//Background
	hEtaBack->SetLineColor(kBlue); 		//Fit Color
	hEtaBack->SetLineStyle(kDashed);		//Fit Style
	hEtaBack->Draw("same");

	//Signal
	hEtaSig->SetLineColor(kMagenta); 		//Fit Color
	hEtaSig->Draw("same");

	//Select canvas part
	c3->cd(2);

	//Signal
	hEtaSig->SetLineColor(kMagenta); 		//Fit Color
	hEtaSig->Draw("same");



	//Saves as image
	c1->SaveAs("InvariantMassProbe.png");
	c2->SaveAs("PtProbe.png");
	c3->SaveAs("EtaProbe.png");



	//SHow chi-squared test (on prompt)
	cout << endl;
	cout << "Chi2/ndf = " << f->GetChisquare()/f->GetNDF() << endl;
	cout << endl;
	printf("Particles (Signal) = %.1f +- %.1f\n",S,dS);
}

//Calls functions to draw and save invariant mass histogram
void bigboss() {
	invariantMassProbe();	//Draws and save invariant mass histogram for pp collision
}