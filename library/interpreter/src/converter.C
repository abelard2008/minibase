#include <assert.h>
#include <string.h>
#include "query.h"
#include "iterator.h"
#include "interpreter.h"

int Converter::convert(AttributeList project, FldSpec outFlds[]){
	Attribute* attPtr;
	int i = 0;
	for(project.GoHead(); (attPtr = project.CurrPtr()); project.GoNext()){
		convert(attPtr, outFlds[i]);
		i++;
	}
	return i;
}

void Converter::convert(Attribute* attrib, FldSpec& fld){
	Attribute* attPtr;
	int i = 1;
	for(al1.GoHead(); (attPtr = al1.CurrPtr()); al1.GoNext()){
		if(attPtr->FullName() == attrib->FullName()){
			fld.relation = outer;
			fld.offset = i;
			return;
		}
		i++;
	}
	i = 1;
	for(al2.GoHead(); (attPtr = al2.CurrPtr()); al2.GoNext()){
		if(attPtr->FullName() == attrib->FullName()){
			fld.relation = innerRel;
			fld.offset = i;
			return;
		}
		i++;
	}
	assert(0);
}

CondExpr* Converter::convert(SelectElem* selEl, CondExpr* next){
	AttrValue val;
	CondExpr* retval = new CondExpr();
	retval->next = next;
	retval->op = selEl->OldGetOperator();
	if(selEl->OldArg1()->IsLiteral()){
		val = selEl->OldArg1()->GetLiteral();
		retval->type1 = val.Type();
		switch(val.Type()){
		case attrInteger:
			retval->operand1.integer = val.GetInteger();
			break;
		case attrReal:
			retval->operand1.real = val.GetReal();
			break;
		case attrString:
                  {
                    const char* tmp = val.GetString().c_str();
			retval->operand1.string = new char[strlen(tmp) + 1];
			strcpy(retval->operand1.string, tmp);
			break;
                  }
		default:
			assert(0);
		}
	}
	else{
		retval->type1 = attrSymbol;
		convert(selEl->OldArg1()->AttributePtr(), retval->operand1.symbol); 
	}
	if(selEl->OldArg2()->IsLiteral()){
		val = selEl->OldArg2()->GetLiteral();
		retval->type2 = val.Type();
		switch(val.Type()){
		case attrInteger:
			retval->operand2.integer = val.GetInteger();
			break;
		case attrReal:
			retval->operand2.real = val.GetReal();
			break;
		case attrString:
                  {
                    const char* tmp = val.GetString().c_str();
			retval->operand2.string = new char[strlen(tmp) + 1];
			strcpy(retval->operand2.string, tmp);
			break;
                  }
		default:
			assert(0);
		}
	}
	else{
		retval->type2 = attrSymbol;
		convert(selEl->OldArg2()->AttributePtr(), retval->operand2.symbol); 
	}
	return retval;
}

CondExpr* Converter::convert(SelectTerm* selTm, CondExpr* next){
	
	// Assumes that the op is or

	return convert(selTm->Sel1(), convert(selTm->Sel2(), next));
}

CondExpr* Converter::convert(SelectExpr* selEx, CondExpr* next){
	if(selEx->IsElement()){
		return convert(selEx->GetElem(), next);
	}
	else{
		return convert(selEx->GetTerm(), next);
	}
}

int Converter::convert(SelectList selLst, CondExpr* selects[]){
	Select* selPtr;
	int i = 0;
	int primPos = 0;
	for(selLst.GoHead(); (selPtr = selLst.CurrPtr()); selLst.GoNext()){
		if(selPtr == plan->PrimaryJoinPredicate()){
			primPos = i;
		}
		selects[i] = convert(*selPtr, NULL);
		i++;
	}
	selects[i] = NULL;
	if(primPos != 0){		 // Switch positions
		CondExpr* tmp = selects[0];
		selects[0] = selects[primPos];
		selects[primPos] = tmp;
	}
	return i;
}

int Converter::convert(
	SelectList /*selLst*/, CondExpr* selects[], int& outerCol, int& innerCol){
	Select* selPtr;
	int i = 0;
	plan->predicate.GoHead(); 
	while( (selPtr = plan->predicate.CurrPtr()) ){
		if(selPtr == plan->PrimaryJoinPredicate()){
			CondExpr* tmpSel = convert(*selPtr, NULL);	
			if(tmpSel->operand1.symbol.relation == outer){
				outerCol = tmpSel->operand1.symbol.offset;
				innerCol = tmpSel->operand2.symbol.offset;
			}
			else{
				outerCol = tmpSel->operand2.symbol.offset;
				innerCol = tmpSel->operand1.symbol.offset;
			}
		}
		else{
			selects[i] = convert(*selPtr, NULL);
			i++;
		}
		plan->predicate.GoNext();
	}
	selects[i] = NULL;
	return i;
}

int Converter::convert(AttrType types[], short* strLens, RelSpec rel){
	int i = 0;
	int j = 0;
	Attribute* attribPtr;
	AttributeList tmp;
	if(rel == outer)
		tmp = al1;
	else
		tmp = al2;
	for(tmp.GoHead(); (attribPtr = tmp.CurrPtr()); tmp.GoNext()){
		//assert(i < MAX_ATTR_LIST);
		types[i] = attribPtr->Type();
		if(types[i] == attrString){
			strLens[j++] = (short) attribPtr->Size();
		}
		i++;
	}
	return i;
}

void Converter::convert(RelSpec rel, PlanNode* child, int column, 
	int& fldLen, TupleOrder& ord){
	int i = 1;
	Attribute* attribPtr;
	AttributeList tmp;
	if(rel == outer)
		tmp = al1;
	else
		tmp = al2;
	for(tmp.GoHead(); (attribPtr = tmp.CurrPtr()); tmp.GoNext()){
		//assert(i < MAX_ATTR_LIST);
		if(i == column){
			if(attribPtr->Type() == attrString){
				fldLen = attribPtr->Size();
			}
			else if(attribPtr->Type() == attrInteger){
				fldLen = sizeof(int);
			}
			else if(attribPtr->Type() == attrReal){
				fldLen = sizeof(float);
			}
			else{
				std::cout << "Unknown type\n";
				assert(0);
			}
			TupleOrder childOrd = child->GetAccessMethod().Order();
			Attribute* ordAttPtr = child->GetAccessMethod().AttributePtr();
			if(childOrd == Random){
				ord = Random;
			}
			else{
				assert(ordAttPtr);
				if(attribPtr->FullName() == ordAttPtr->FullName()){
					ord = childOrd;
				}
				else{
					ord = Random;
				}
			}
			break;
		}
		i++;
	}
}

void Converter::get(int& noInFlds, AttrType*& types, short*& strLens, 
	CondExpr**& selects, FldSpec*& projects, int& noOutFlds){

     projects = new FldSpec[MAX_ATTR_LIST];
	selects = new CondExpr*[MAX_ATTR_LIST];
	types = new AttrType[MAX_ATTR_LIST];
	strLens = new short[MAX_ATTR_LIST];

	noInFlds = convert(types, strLens);
	noOutFlds = convert(plan->project, projects);
	convert(plan->predicate, selects);
	assert(noInFlds);
	assert(noOutFlds);
}

void Converter::get(
	int& memory, int& noOuterFlds,
	AttrType*& outerTypes, short*& outStrLens, CondExpr**& selects,
	FldSpec*& projects, int& noResFlds
	){

     projects = new FldSpec[MAX_ATTR_LIST];
	selects = new CondExpr*[MAX_ATTR_LIST];
	outerTypes = new AttrType[MAX_ATTR_LIST];
	outStrLens = new short[MAX_ATTR_LIST];

	memory = plan->MemoryRequired();
	noOuterFlds = convert(outerTypes, outStrLens, outer);
	noResFlds = convert(plan->project, projects);
	convert(plan->predicate, selects);
	assert(noOuterFlds);
	assert(noResFlds);
}

void Converter::getINL(
	int& memory, int& noOuterFlds, 
	AttrType*& outerTypes, short*& outerStrLens, CondExpr**& selects,
	FldSpec*& projects, int& noResFlds, int& outerCol, int& innerCol
	){

     projects = new FldSpec[MAX_ATTR_LIST];
	selects = new CondExpr*[MAX_ATTR_LIST];
	outerTypes = new AttrType[MAX_ATTR_LIST];
	outerStrLens = new short[MAX_ATTR_LIST];

	memory = plan->MemoryRequired();
	noOuterFlds = convert(outerTypes, outerStrLens, outer);
	noResFlds = convert(plan->project, projects);
	convert(plan->predicate, selects, outerCol, innerCol);
	assert(noOuterFlds);
	assert(noResFlds);
}

/************ old get, might be used in dual iterators

void Converter::get(
	int& noInnerFlds, int& noOuterFlds, AttrType*& innerTypes,
	AttrType*& outerTypes, CondExpr**& selects,
	FldSpec*& projects, int& noResFlds
	){

     projects = new FldSpec[MAX_ATTR_LIST];
	selects = new (CondExpr*)[MAX_ATTR_LIST];
	innerTypes = new AttrType[MAX_ATTR_LIST];
	outerTypes = new AttrType[MAX_ATTR_LIST];
	strLens = new short[MAX_ATTR_LIST];

	noInnerFlds = convert(innerTypes, innerRel);
	noOuterFlds = convert(outerTypes, outer);
	noResFlds = convert(plan->project, projects);
	convert(plan->predicate, selects);
	assert(noInnerFlds);
	assert(noOuterFlds);
	assert(noResFlds);
}
****************/
/*	primary predicate and tuple order:

	 if (predicate.CurrPtr() == primaryJoinPredicate)
		 std::cout << " (primary)";

	if (project.CurrPtr() == accessMethod.AttributePtr() &&
		accessMethod.Order() != Random)
		switch (accessMethod.Order()) {
			  case Ascending: std::cout << "  (Ascending)"; break;
		    case Descending: std::cout << "  (Descending)"; break;

	attr1 = primaryJoinPredicate->GetExpr()->GetElem()->OldArg1()->AttributePtr();
	if(attr1 == leftchild->accessMethod.AttributePtr()){
		leftorder = leftchild->accesMethod.Order();
	}
	else{
		leftorder = Random;
	}

(gdb) p * child->GetAccessMethod().AttributePtr()
$4 = {name = {rep = 0x102af8}, attrId = 31, relation = 0x1027c8,
type = attrInteger, size = 2, offset = 0, minVal = {type = attrInteger, s = {
rep = 0xec980},  = {i = 1, r = 2.1219957909652723e-314}}, maxVal = {
type = attrInteger, s = {rep = 0xec980},  = {i = 999,
r = 2.119873795174307e-311}},
additionalProperties = {<DList<PropertyNode>> = {<ListBase<PropertyNode>> = {
head = 0x0, tail = 0x0, curr = 0x0, count = 0}, }, dConstructor = 0}}

*/
