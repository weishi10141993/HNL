// Muon
#include "DataFormats/PatCandidates/interface/Muon.h"

// tracks
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

// vertex
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BTauReco/interface/VertexTypes.h"

// kinematics
#include "DataFormats/Math/interface/deltaR.h"

class VertexAssociation {
 public:
  VertexAssociation(std::string nameString, const reco::Vertex & PV, const int& debug_) { 
    primaryVertex   = PV;
    name	    = nameString;
    debug           = debug_;
  }

  // fillers
  //void addMuon( const  pat::MuonCollection & muon);
  void addVertex( const reco::Vertex & vertex);
  void setPrimaryVertex(const reco::Vertex & pv){ primaryVertex = pv; };
  
  // accessors
  int	getNVertices(){ return vertexCollection.size(); }
  //int   getNMuon() { return MuonCollection.size(); }
  float getBestVertexScore() { return bestVertexScore; }

  std::string getName() { return name; }
  // score
  const std::pair<const reco::Vertex, const float> getBestVertex(edm::Handle< pat::MuonCollection >&, const std::string&);
  float		getVertexScore(edm::Handle< pat::MuonCollection >&, const reco::Vertex & , const std::string &);    
  
 private:
  reco::Vertex		    primaryVertex;
  //pat::MuonCollection       muonCollection;
  reco::VertexCollection    vertexCollection;    
  reco::Vertex   	    bestVertex;
  float			    bestVertexScore;  
  std::string		    name;
  int			    debug;
};
//void VertexAssociation::addMuon(const  pat::MuonCollection & muon) { muonCollection.push_back(muon); }
void VertexAssociation::addVertex(const  reco::Vertex & vertex) { vertexCollection.push_back(vertex); }
float VertexAssociation::getVertexScore(edm::Handle< pat::MuonCollection >& muon, const reco::Vertex & vertex, const std::string & algo) {
  float  score   = 0;
  double muon_pt = 999;
  double muon_eta = 999;
  double muon_phi = 999;
  pat::MuonCollection::const_iterator mu = muon->begin();
  for(; mu != muon->end(); ++mu) {
    reco::TrackRef tunePTrack = mu->muonBestTrack();
    if(tunePTrack->pt() < muon_pt && 
       tunePTrack->pt() > 5 && 
       mu->isGlobalMuon() && 
       mu->isPFMuon() && 
       fabs(tunePTrack->eta()) < 2.4){
      muon_pt  = tunePTrack->pt();
      muon_eta = tunePTrack->eta();
      muon_phi = tunePTrack->phi();
    }
  }
  if (debug > 3)   std::cout << "[DEBUG 3] [SVA] Muon pT=" << muon_pt << " Muon Eta: " << muon_eta <<" Muon Phi "<< muon_phi << std::endl;
  if (debug > 3) std::cout << "[DEBUG 3] [SVA] iterating over tracks" << std::endl;
  reco::Vertex::trackRef_iterator tt = vertex.tracks_begin();
  for(; tt != vertex.tracks_end(); ++tt) {
    float   eta = (*tt)->eta();
    float   phi = (*tt)->phi();    
    if (debug > 4)    std::cout << "[DEBUG 4] [SVA] Track Eta=" << eta << " Phi: " << phi <<  std::endl;
    float   dR		   = reco::deltaR(muon_eta, muon_phi, eta, phi);
    if (dR > 1) // was 0.4
      continue;
    else 
      score += 1.0 / dR;	 	  	 
  } 
  if (debug > 3) std::cout << "[DEBUG 3] [SVA] Vertex Matching Score: " << score <<  std::endl;
  return score;
}

const std::pair<const reco::Vertex, const float> VertexAssociation::getBestVertex(edm::Handle<pat::MuonCollection>& muon, const std::string & algo) {
  // keep track of the best vertex
  float		bestScore = -1;
  reco::Vertex	bestVertex;

  if (debug > 1) std::cout << "[DEBUG 1] [SVA] iterating over Vertices" << std::endl;
  reco::VertexCollection::const_iterator ss = vertexCollection.begin();
  for(; ss != vertexCollection.end(); ++ss) {
    float score = getVertexScore(muon, *ss, algo);
    
    if (score > bestScore) {  // was bestScore
      bestScore	 = score;
      bestVertex = *ss;
    }    
  }

  if (bestScore > 0) {
    const std::pair<const reco::Vertex, const float> vertexPair(bestVertex, bestScore);
    return vertexPair;  // we find at least one vertex with a track within dR = 1.0
  }

  const std::pair<const reco::Vertex, const float> vertexPair(primaryVertex, 0);
  return vertexPair;
}
