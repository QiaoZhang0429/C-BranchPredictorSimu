//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//

#include "predictor.h"
//#include <math.h>
#include <memory.h>

//
// Student Information
//
const char *studentName = "Qiao Zhang";
const char *studentID   = "A53095965";
const char *email       = "qiz121@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[5] = { "Static", "Gshare", "Local",
                          "Tournament", "Custom" };

#define SG  0     // predict global predictor
#define WG  1     // predict global predictor
#define WL  2     // predict local predictor
#define SL  3     // predict local predictor

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t ghr;
uint8_t *BHT;
uint32_t *PHT;
uint32_t globalHistory;
uint8_t *globalBHT;
uint32_t *localPHT;
uint8_t *localBHT;
uint8_t *choice;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

void gshareInit();
uint8_t gsharePrediction(uint32_t pc); 
void gshareTrain(uint32_t pc, uint8_t outcome);

void localInit();
uint8_t localPrediction(uint32_t pc); 
void localTrain(uint32_t pc, uint8_t outcome);

void tournamentInit();
uint8_t tournamentPrediction(uint32_t pc); 
void tournamentTrain(uint32_t pc, uint8_t outcome);

void customInit();
uint8_t customPrediction(uint32_t pc); 
void customTrain(uint32_t pc, uint8_t outcome);

double powTwo(int x, int n);

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      gshareInit();
      break;
    case LOCAL:
      localInit();
      break;
    case TOURNAMENT:
      tournamentInit();
      break;
    case CUSTOM:
      customInit();
      break;
    default:
      break;
  }

}

void 
gshareInit() 
{
  ghr = 0;
  BHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  memset(BHT, WN, sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
}

void 
localInit()
{
  PHT = (uint32_t*) malloc(sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  memset(PHT, 0, sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  BHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
  memset(BHT, WN, sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
}

void 
tournamentInit()
{
  globalHistory = 0;
  globalBHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  memset(globalBHT, WN, sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  localPHT = (uint32_t*) malloc(sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  memset(localPHT, 0, sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  localBHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
  memset(localBHT, WN, sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
  choice = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  memset(choice, WG, sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
}

void 
customInit()
{
  ghistoryBits = 14;
  lhistoryBits = 12;
  pcIndexBits = 12;
  globalHistory = 0;
  globalBHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  memset(globalBHT, WN, sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  localPHT = (uint32_t*) malloc(sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  memset(localPHT, 0, sizeof(uint32_t) * (int)powTwo(2, pcIndexBits));
  localBHT = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
  memset(localBHT, WN, sizeof(uint8_t) * (int)powTwo(2, lhistoryBits));
  choice = (uint8_t*) malloc(sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
  memset(choice, WG, sizeof(uint8_t) * (int)powTwo(2, ghistoryBits));
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gsharePrediction(pc);
    case LOCAL:
      return localPrediction(pc);
    case TOURNAMENT:
      return tournamentPrediction(pc);
    case CUSTOM:
      return customPrediction(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

uint8_t
gsharePrediction(uint32_t pc)
{
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask |= (1 << i);
  }
  uint32_t index = (pc ^ ghr) & mask;
  if(BHT[index] == SN || BHT[index] == WN)
    return NOTTAKEN;
  else
    return TAKEN;
}

uint8_t 
localPrediction(uint32_t pc)
{
  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) {
    pc_mask |= (1 << i);
  }
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = PHT[p_index];

  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) {
    lh_mask |= (1 << i);
  }
  localHistory &= lh_mask;
  if(BHT[localHistory] == SN || BHT[localHistory] == WN)
    return NOTTAKEN;
  else
    return TAKEN;
}

uint8_t 
tournamentPrediction(uint32_t pc)
{
  uint8_t globalPred;
  uint8_t localPred;

  uint32_t g_mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    g_mask |= (1 << i);
  }
  uint32_t g_index = globalHistory & g_mask;
  if(globalBHT[g_index] == SN || globalBHT[g_index] == WN)
    globalPred = NOTTAKEN;
  else
    globalPred = TAKEN;

  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) {
    pc_mask |= (1 << i);
  }
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = localPHT[p_index];
  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) {
    lh_mask |= (1 << i);
  }
  localHistory &= lh_mask;
  if(localBHT[localHistory] == SN || localBHT[localHistory] == WN)
    localPred = NOTTAKEN;
  else
    localPred = TAKEN;

  if(choice[g_index] == SG || choice[g_index] == WG) {
    return globalPred;
  }
  else
    return localPred;
}

uint8_t 
customPrediction(uint32_t pc)
{
  uint8_t globalPred;
  uint8_t localPred;

  uint32_t g_mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    g_mask |= (1 << i);
  }
  uint32_t g_index = (globalHistory ^ pc) & g_mask;
  uint32_t c_index = g_index;//globalHistory & g_mask;
  if(globalBHT[g_index] == SN || globalBHT[g_index] == WN)
    globalPred = NOTTAKEN;
  else
    globalPred = TAKEN;

  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) {
    pc_mask |= (1 << i);
  }
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = localPHT[p_index];
  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) {
    lh_mask |= (1 << i);
  }
  localHistory &= lh_mask;
  if(localBHT[localHistory] == SN || localBHT[localHistory] == WN)
    localPred = NOTTAKEN;
  else
    localPred = TAKEN;

  if(choice[c_index] == SG || choice[c_index] == WG) {
    return globalPred;
  }
  else
    return localPred;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      gshareTrain(pc, outcome);
      break;
    case LOCAL:
      localTrain(pc, outcome);
      break;
    case TOURNAMENT:
      tournamentTrain(pc, outcome);
      break;
    case CUSTOM:
      customTrain(pc, outcome);
      break;
    default:
      break;
  }
}

void 
gshareTrain(uint32_t pc, uint8_t outcome)
{
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) 
    mask |= (1 << i);
  uint32_t index = (pc ^ ghr) & mask;
  if(outcome == TAKEN && BHT[index] != ST) 
    BHT[index]++;
  else if(outcome == NOTTAKEN && BHT[index] != SN) 
    BHT[index]--;
  ghr = ghr << 1;
  if(outcome == TAKEN) 
    ghr |= (uint32_t)1;
}

void 
localTrain(uint32_t pc, uint8_t outcome)
{
  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) 
    pc_mask |= (1 << i);
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = PHT[p_index];

  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) 
    lh_mask |= (1 << i);
  localHistory &= lh_mask;
  if(outcome == TAKEN && BHT[localHistory] != ST) 
    BHT[localHistory]++;
  else if(outcome == NOTTAKEN && BHT[localHistory] != SN) 
    BHT[localHistory]--;
  PHT[p_index] = PHT[p_index] << 1;
  if(outcome == TAKEN) 
    PHT[p_index] |= (uint32_t)1;
}

void 
tournamentTrain(uint32_t pc, uint8_t outcome)
{
  uint8_t globalPred;
  uint8_t localPred;

  uint32_t g_mask = 0;
  for(int i = 0; i < ghistoryBits; i++) 
    g_mask |= (1 << i);
  uint32_t g_index = globalHistory & g_mask;
  if(globalBHT[g_index] == SN || globalBHT[g_index] == WN)
    globalPred = NOTTAKEN;
  else
    globalPred = TAKEN;
  if(outcome == TAKEN && globalBHT[g_index] != ST) 
    globalBHT[g_index]++;
  else if(outcome == NOTTAKEN && globalBHT[g_index] != SN) 
    globalBHT[g_index]--;

  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) 
    pc_mask |= (1 << i);
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = localPHT[p_index];
  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) 
    lh_mask |= (1 << i);
  localHistory &= lh_mask;
  if(localBHT[localHistory] == SN || localBHT[localHistory] == WN)
    localPred = NOTTAKEN;
  else
    localPred = TAKEN;
  if(outcome == TAKEN && localBHT[localHistory] != ST) 
    localBHT[localHistory]++;
  else if(outcome == NOTTAKEN && localBHT[localHistory] != SN) 
    localBHT[localHistory]--;

  if(globalPred != localPred) {
    if(outcome == globalPred && choice[g_index] != SG)
      choice[g_index]--;
    else if(outcome == localPred && choice[g_index] != SL)
      choice[g_index]++;
  }

  globalHistory = globalHistory << 1;
  if(outcome == TAKEN) 
    globalHistory |= (uint32_t)1;

  localPHT[p_index] = localPHT[p_index] << 1;
  if(outcome == TAKEN) 
    localPHT[p_index] |= (uint32_t)1;
}

void 
customTrain(uint32_t pc, uint8_t outcome)
{
  uint8_t globalPred;
  uint8_t localPred;

  uint32_t g_mask = 0;
  for(int i = 0; i < ghistoryBits; i++) 
    g_mask |= (1 << i);
  uint32_t g_index = (globalHistory ^ pc) & g_mask;
  if(globalBHT[g_index] == SN || globalBHT[g_index] == WN)
    globalPred = NOTTAKEN;
  else
    globalPred = TAKEN;
  if(outcome == TAKEN && globalBHT[g_index] != ST) 
    globalBHT[g_index]++;
  else if(outcome == NOTTAKEN && globalBHT[g_index] != SN) 
    globalBHT[g_index]--;

  uint32_t pc_mask = 0;
  for(int i = 0; i < pcIndexBits; i++) 
    pc_mask |= (1 << i);
  uint32_t p_index = pc & pc_mask;
  uint32_t localHistory = localPHT[p_index];
  uint32_t lh_mask = 0;
  for(int i = 0; i < lhistoryBits; i++) 
    lh_mask |= (1 << i);
  localHistory &= lh_mask;
  if(localBHT[localHistory] == SN || localBHT[localHistory] == WN)
    localPred = NOTTAKEN;
  else
    localPred = TAKEN;
  if(outcome == TAKEN && localBHT[localHistory] != ST) 
    localBHT[localHistory]++;
  else if(outcome == NOTTAKEN && localBHT[localHistory] != SN) 
    localBHT[localHistory]--;

  uint32_t c_index = g_index;//globalHistory & g_mask;
  if(globalPred != localPred) {
    if(outcome == globalPred && choice[c_index] != SG)
      choice[c_index]--;
    else if(outcome == localPred && choice[c_index] != SL)
      choice[c_index]++;
  }

  globalHistory = globalHistory << 1;
  if(outcome == TAKEN) 
    globalHistory |= (uint32_t)1;

  localPHT[p_index] = localPHT[p_index] << 1;
  if(outcome == TAKEN) 
    localPHT[p_index] |= (uint32_t)1;
}

double 
powTwo(int x, int n)
{
  if(n == 0)
    return 1.0;
  double ans = 1.0 ;  
  while(n > 0) {  
    if((n & 1) > 0)  
      ans *= x; 
    x *= x;
    n >>= 1;
  }  
  return ans; 
}