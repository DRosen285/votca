/* 
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <vector>
#include <boost/lexical_cast.hpp>
#include <votca/tools/getline.h>
#include <votca/tools/elements.h>
#include "lammpsdatareader.h"

namespace votca { namespace csg {
	using namespace boost;
	using namespace std;


	/*****************************************************************************
	 * Public Facing Methods                                                     *
	 *****************************************************************************/

	bool LAMMPSDataReader::ReadTopology(string file,  Topology &top)
	{
		topology_ = true;
		top.Cleanup();

		fl_.open(file.c_str());
		if(!fl_.is_open())
			throw std::ios_base::failure("Error on open topology file: " + file);
		fname_=file;

		NextFrame(top);

		fl_.close();

		return true;
	}

	bool LAMMPSDataReader::Open(const string &file)
	{
		fl_.open(file.c_str());
		if(!fl_.is_open())
			throw std::ios_base::failure("Error on open trajectory file: " + file);
		fname_=file;
		return true;
	}

	void LAMMPSDataReader::Close()
	{
		fl_.close();
	}

	bool LAMMPSDataReader::FirstFrame(Topology &top)
	{
		topology_ = false;
		NextFrame(top);
		return true;
	}

	bool LAMMPSDataReader::NextFrame(Topology &top)
	{

		int timestep = -1;
		string line;
		getline(fl_, line);

		while(!fl_.eof()) {

			bool labelmatch = false;
			Tokenizer tok(line, " ");
			tok.ToVector(fields);

			// If not check the size of the vector and parse according 
			// to the number of fields
			if(fields.size()==1){
				labelmatch = matchOneFieldLabel(fields,top);
			}else if(fields.size()==2){
				labelmatch = matchTwoFieldLabels(fields,top);
			}else if(fields.size()==3){
				labelmatch = matchThreeFieldLabels(fields,top);
			}else if(fields.size()==4){
				labelmatch = matchFourFieldLabels(fields,top);
			}else{

				// See if the line is the lammps .data header/info line
				labelmatch = matchFieldsTimeStepLabel(fields,top);

				if(!labelMatched){
					string err = "Unrecognized line in lammps .data file:\n"+line;
					throw runtime_error(err);
				}
			}
			getline(fl_, line);
		}
		return !fl_.eof();;
	}

	/*****************************************************************************
	 * Private Facing Methods                                                    *
	 *****************************************************************************/

	bool LAMMPSDataReader::matchOneFieldLabel_(
			vector<string> fields,
			Topology & top){

		if(fields.at(0) == "Masses"){
			SortIntoDataGroup_("Masses");
			InitializeAtomTypes_();
		}else if(fields.at(0) == "Atoms" ){
			ReadAtoms_(Topology &top) {
			}else if(fields.at(0) == "Bonds" ){
				ReadBonds_(Topology &top) {
				}else if(fields.at(0) == "Angles" ){
					ReadAngles_(Topology &top) {
					}else if(fields.at(0) == "Dihedrals"){
						ReadDihedrals_(Topology &top) {
						}else if(fields.at(0) == "Impropers"){
						}else{
							return false;
						}
						return true;
					}

					bool matchTwoFieldLabels_(vector<string> fields,Topology & top){

						string label = fields.at(0)+" "+fields.at(1);

						if(fields.at(1) == "atoms"){
							ReadNumberOfAtoms_(fields);
						}else if(fields.at(1) == "bonds" ){
							ReadNumberOfBonds_(fields);
						}else if(fields.at(1) == "angles"){
							ReadNumberOfAngles_(fields);
						}else if(fields.at(1) == "dihedrals"){
							ReadNumberOfDihedrals_(fields);
						}else if(fields.at(1) == "impropers"){
							ReadNumberOfImpropers_(fields);
						}else if(label == "Pair Coeffs"){
							SortIntoDataGroup_("Pair Coeffs")
						}else if(label == "Bond Coeffs"){
							SortIntoDataGroup_("Bond Coeffs")
						}else if(label == "Angle Coeffs"){
							SortIntoDataGroup_("Angle Coeffs")
						}else if(label == "Improper Coeffs"){
							SortIntoDataGroup_("Improper Coeffs")
						}else{
							return false;
						}
						return true;
					}

					bool matchThreeFieldLabels_(vector<string> fields,Topology & top){
						string label = fields.at(1)+" "+fields.at(2);
						if(label == "atom types"){
							readNumberTypes_(fields,"atom");
						}else if(label == "bond types" ){
							readNumberTypes_(fields,"bond");
						}else if(label == "angle types"){
							readNumberTypes_(fields,"angle");
						}else if(label == "dihedral types"){
							readNumberTypes_(fields,"Dihedral");
						}else if(label == "improper types"){
							readNumberTypes_(fields,"Improper");
						}else{
							return false;
						}
						return true;
					}

					bool matchFourFieldLabels_(vector<string> fields,Topology & top){
						string label = fields.at(2)+" "+fields.at(3);
						if(label == "xlo xhi"){
							ReadBox_(fields,top);
						}else{
							return false;
						}
						return true;
					}

					bool matchFieldsTimeStepLabel_(vector<string> fields,Topology & top){
						for(auto word_ptr=tok.begin(); word_ptr!=tok.end(); ++word_ptr) {
							if(*word == "timestep"){
								top.setStep(boost::lexical_cast<int>(*(word_ptr+2));
										cout << "Reading frame, timestep " << top.getStep() << endl;
										return true;
										}
										}
										return false;
										}

										// The purpose of this function is to take lammps output where there are more
										// than a single atom type of the same element. For instance there may be 4
										// atom types with mass of 12.01. Well this means that they are all carbon but
										// are treated differently in lammps. It makes since to keep track of this. So
										// When creating the atom names we will take into account so say we have the
										// following masses in the lammps .data file:
										// Masses
										// 
										// 1 1.0
										// 2 12.01
										// 3 12.01
										// 4 16.0
										// 5 12.01
										//
										// Then we would translate this to the following atom names
										// 1 H
										// 2 C1
										// 3 C2
										// 4 O
										// 5 C3
										//
										// Note that we do not append a number if it is singular, in such cases the 
										// element and the atom name is the same. 
					void LAMMPSDataReader::InitializeAtomTypes_(){
						if(!data_.count("Masses")){
							string err = "Masses must first be parsed before the atoms can be read."; 
							throw runtime_error(err);
						}

						std::map<std::string, int> countAtomsOfSameElement;
						Elements elements;

						for( auto mass : data_["Masses"] ){
							// Determine the mass associated with the atom 
							double mass_atom = lexical_cast<double>(mass);
							// Determine the element (Symbol) by looking at the mass
							// Second argument is the tolerance
							string eleShort = getEleShortClosestInMass(mass_atom,0.01);
							// Count the number of atoms 
							if(!countAtomsOfSameElement.count(eleShort)){
								countAtomsOfSameElement[eleShort] = 1;		
							}
							countAtomsOfSameElement[eleShort]++;		
						}

						vector<int> indices(0,countAtomsOfSameElement.size());	
						// If there is more than one atom type of the same element append a number
						// to the atom type name
						map<string,int> eleShortIndices;
						int index=0;
						for( auto mass : data_["Masses"] ){
							// Determine the mass associated with the atom 
							double mass_atom = lexical_cast<double>(mass);
							// Determine the element (Symbol) by looking at the mass
							// Second argument is the tolerance
							string eleShort = getEleShortClosestInMass(mass_atom,0.01);
							string label = eleShort;
							if(countAtomsOfSameElement[eleShort]>1){
								if(eleShortIndices.count(eleShort)==0){
									label+="1";
									eleShortIndices[eleShort]=1;
								}else{
									eleShortIndices[eleShort]++;	
									label+=to_string(eleShortIndices[eleShort]);
								}
							}
							atomtypes_[index].push_back(eleShort);
							atomtypes_[index].push_back(lebel);	
							++index;
						}
					}

					void LAMMPSDataReader::ReadBox_(vector<string> fields,Topology & top)
					{
						matrix m;
						m.ZeroMatrix();
						m[0][0] = fields.at(1)-fields.at(2); 

						for(int i=1; i<3; ++i) {
							string line;
							getline(fl_, line);
							Tokenizer tok(line, " ");
							tok.ConvertToVector(fields);
							if(fields.size()!=4){
								throw runtime_error("invalid box format in the lammps data file");
							}
							m[i][i] = fields.at(1)-fields.at(2);
						}
						top.setBox(m);
					}

					void LAMMPSDataReader::SortIntoDataGroup_(string tag){
						string line;
						getline(fl_,line);
						getline(fl_,line);

						vector<vector<string>> group;		
						string data_elem;
						while(line!=""){
							vector<string> mini_group;
							istringstream iss(line);
							iss >> data_elem;
							while(!iss.str().empty()){
								iss >> data_elem;
								mini_group.push_back(data_elem);
							}
							group.push_back(mini_group);
							getline(fl_,line);
						}
						data_[tag] = group;
					}

					void LAMMPSDataReader::ReadNumTypes_(vector<string> fields, string type){
						numberOfDifferentTypes_[type] = boost::lexical_cast<int>(fields.at(0));
					}

					void LAMMPSDataReader::ReadNumAtoms_(vector<string> fields, Topology &top){
						numberOf_["atoms"] = boost::lexical_cast<int>(fields.at(0));
						if(!topology_ && numberOf_["atoms"] !=top.BeadCount())
							std::runtime_error("number of beads in topology and trajectory differ");
					}

					void LAMMPSDataReader::ReadNumBonds_(vector<string> fields){
						numberOf_["bonds"] = boost::lexical_cast<int>(fields.at(0));
					}

					void LAMMPSDataReader::ReadNumAngles_(vector<string> fields){
						numberOf_["angles"] = boost::lexical_cast<int>(fields.at(0));
					}

					void LAMMPSDataReader::ReadNumDihedrals_(vector<string> fields){
						numberOf_["dihedrals"] = boost::lexical_cast<int>(fields.at(0));
					}

					void LAMMPSDataReader::ReadNumImpropers_(vector<string> fields){
						numberOf_["impropers"] = boost::lexical_cast<int>(fields.at(0));
					}

					void LAMMPSDataReader::ReadAtoms_(Topology &top) {

						string line;
						getline(fl_,line);
						getline(fl_,line);

						int atomId;
						int moleculeId;
						int atomTypeId;

						double charge;
						double x, y, z;

						while(line!=""){
							istringstream iss(line);
							iss >> atomId;
							iss >> moleculeId;
							iss >> atomTypeId;
							iss >> charge;
							iss >> x;
							iss >> y;
							iss >> z;

							Molecule * mol;
							if(!molecules_.count(moleculeId)){
								mol = top.CreateMolecule("Unknown");
							}else{
								mol = molecules_[moleculeId];
							}

							string bead_type_name;
							int symmetry = 1; // spherical
							BeadType * bead_type = top.GetOrCreateBeadType();

							// Will use the molecule id as the resnum for lack of a better option	
							resnr = moleculeId;

							if(atomtypes_.count(atomTypeId)==0){
								string err = "Unrecognized atomTypeId, the atomtypes map "
									"may be uninitialized";
								throw runtime_error(err);
							}
							string beadname = atomtypes_[atomTypeId].at(2);	

							Bead * b = top.CreateBead(
									symmetry,
									beadname,
									bead_type,
									resnr,
									mass,
									charge);

							getline(fl_,line);
						}

					}

					void LAMMPSDataReader::ReadBonds_(Topology &top) {
						string line;
						getline(fl_,line);
						getline(fl_,line);

						int bondId;
						int bondTypeId;
						int atom1Id, atom2Id;

						while(line!=""){
							istringstream iss(line);
							iss >> bondId;
							iss >> bondTypeId;
							iss >> atom1Id;
							iss >> atom2Id;

							Interaction * ic = new IBond(atom1Id,atom2Id);
							ic->setGroup("BONDS");
							ic->setIndex(bondId);
							auto b = top.getBead(atom1Id);
							auto mi = b->getMolecule();
							ic->setMolecule(mi);
							top.AddBondedInteraction(ic);
							mi->AddInteraction(ic);

						}
					}

					void LAMMPSDataReader::ReadAngles_(Topology &top) {
						string line;
						getline(fl_,line);
						getline(fl_,line);

						int angleId;
						int angleTypeId;
						int atom1Id, atom2Id, atom3Id;

						while(line!=""){
							istringstream iss(line);
							iss >> bondId;
							iss >> bondTypeId;
							iss >> atom1Id;
							iss >> atom2Id;
							iss >> atom3Id;

							Interaction * ic = new IAngle(atom1Id,atom2Id,atom3Id);
							ic->setGroup("ANGLES");
							ic->setIndex(angleId);
							auto b = top.getBead(atom1Id);
							auto mi = b->getMolecule();
							ic->setMolecule(mi);
							top.AddBondedInteraction(ic);
							mi->AddInteraction(ic);

						}
					}

					void LAMMPSDataReader::ReadDihedrals_(Topology &top) {
						string line;
						getline(fl_,line);
						getline(fl_,line);

						int dihedralId;
						int dihedralTypeId;
						int atom1Id, atom2Id, atom3Id, atom4Id;

						while(line!=""){
							istringstream iss(line);
							iss >> bondId;
							iss >> bondTypeId;
							iss >> atom1Id;
							iss >> atom2Id;
							iss >> atom3Id;
							iss >> atom4Id;

							Interaction * ic = new IDihedral(atom1Id,atom2Id,atom3Id,atom4Id);
							ic->setGroup("DIHEDRALS");
							ic->setIndex(dihedralId);
							auto b = top.getBead(atom1Id);
							auto mi = b->getMolecule();
							ic->setMolecule(mi);
							top.AddBondedInteraction(ic);
							mi->AddInteraction(ic);

						}
					}

				}}

