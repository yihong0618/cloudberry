//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		gpdbwrappers.h
//
//	@doc:
//		Definition of GPDB function wrappers
//
//	@test:
//
//
//---------------------------------------------------------------------------
#ifndef GPDB_gpdbwrappers_H
#define GPDB_gpdbwrappers_H

extern "C" {
#include "postgres.h"

#include "access/amapi.h"
#include "access/attnum.h"
#include "optimizer/plancat.h"
#include "parser/parse_coerce.h"
#include "statistics/statistics.h"
#include "utils/faultinjector.h"
#include "utils/lsyscache.h"
#include "utils/partcache.h"
}

#include "gpos/types.h"

// fwd declarations
using SysScanDesc = struct SysScanDescData *;
struct TypeCacheEntry;
using Numeric = struct NumericData *;
using HeapTuple = struct HeapTupleData *;
using Relation = struct RelationData *;
struct Value;
using TupleDesc = struct TupleDescData *;
struct Query;
using ScanKey = struct ScanKeyData *;
struct Bitmapset;
struct Plan;
union ListCell;
struct TargetEntry;
struct Expr;
struct ExtTableEntry;
struct ForeignScan;
struct Uri;
struct CdbComponentDatabases;
struct StringInfoData;
using StringInfo = StringInfoData *;
struct LogicalIndexes;
struct ParseState;
struct DefElem;
struct GpPolicy;
struct PartitionSelector;
struct Motion;
struct Var;
struct Const;
struct ArrayExpr;

#include "gpopt/utils/RelationWrapper.h"

// without GP_WRAP try...catch functions
namespace gpdb
{
// convert datum to bool
static inline bool BoolFromDatum(Datum d) 
{
	return DatumGetBool(d);
}

// convert bool to datum
static inline Datum DatumFromBool(bool b)
{
	return BoolGetDatum(b);
}

// convert datum to char
static inline char CharFromDatum(Datum d)
{
	return DatumGetChar(d);
}

// convert char to datum
static inline Datum DatumFromChar(char c)
{
	return CharGetDatum(c);
}

// convert datum to int8
static inline int8 Int8FromDatum(Datum d)
{
	return DatumGetInt8(d);
}

// convert int8 to datum
static inline Datum DatumFromInt8(int8 i8)
{
	return Int8GetDatum(i8);
}

// convert datum to uint8
static inline uint8 Uint8FromDatum(Datum d)
{
	return DatumGetUInt8(d);
}

// convert uint8 to datum
static inline Datum DatumFromUint8(uint8 ui8)
{
	return UInt8GetDatum(ui8);
}

// convert datum to int16
static inline int16 Int16FromDatum(Datum d)
{
	return DatumGetInt16(d);
}

// convert int16 to datum
static inline Datum DatumFromInt16(int16 i16)
{
	return Int16GetDatum(i16);
}

// convert datum to uint16
static inline uint16 Uint16FromDatum(Datum d)
{
	return DatumGetUInt16(d);
}

// convert uint16 to datum
static inline Datum DatumFromUint16(uint16 ui16)
{
	return UInt16GetDatum(ui16);
}

// convert datum to int32
static inline int32 Int32FromDatum(Datum d)
{
	return DatumGetInt32(d);
}

// convert int32 to datum
static inline Datum DatumFromInt32(int32 i32)
{
	return Int32GetDatum(i32);
}

// convert datum to uint32
static inline uint32 Uint32FromDatum(Datum d)
{
	return DatumGetUInt32(d);
}

// convert uint32 to datum
static inline Datum DatumFromUint32(uint32 ui32)
{
	return UInt32GetDatum(ui32);
}

// convert datum to int64
static inline int64 Int64FromDatum(Datum d)
{
	return DatumGetInt64(d);
}

// convert int64 to datum
static inline Datum DatumFromInt64(int64 i64)
{
	return Int64GetDatum(i64);
}

// convert datum to uint64
static inline uint64 Uint64FromDatum(Datum d)
{
	return DatumGetUInt64(d);
}

// convert uint64 to datum
static inline Datum DatumFromUint64(uint64 ui64)
{
	return UInt64GetDatum(ui64);
}

// convert datum to oid
static inline Oid OidFromDatum(Datum d)
{
	return DatumGetObjectId(d);
}

// convert datum to generic object with pointer handle
static inline void *PointerFromDatum(Datum d)
{
	return DatumGetPointer(d);
}

// convert pointer to datum
static inline Datum DatumFromPointer(const void *p)
{
	return PointerGetDatum(p);
}

// convert datum to float4
static inline float4 Float4FromDatum(Datum d)
{
	return DatumGetFloat4(d);
}

// convert datum to float8
static inline float8 Float8FromDatum(Datum d)
{
	return DatumGetFloat8(d);
}

}

namespace gpdb
{

// does an aggregate exist with the given oid
bool AggregateExists(Oid oid);

// add member to Bitmapset
Bitmapset *BmsAddMember(Bitmapset *a, int x);

// next member of Bitmapset
int BmsNextMember(const Bitmapset *a, int prevbit);

// create a copy of an object
void *CopyObject(void *from);

// datum size
Size DatumSize(Datum value, bool type_by_val, int type_len);

bool ExpressionReturnsSet(Node *clause);

// expression type
Oid ExprType(Node *expr);

// expression type modifier
int32 ExprTypeMod(Node *expr);

// expression collation
Oid ExprCollation(Node *expr);

// expression collation - GPDB_91_MERGE_FIXME
Oid TypeCollation(Oid type);

// Byval type get len
void TypLenByVal(Oid typid, int16 *typlen, bool *typbyval);

// extract nodes with specific tag from a plan tree
List *ExtractNodesPlan(Plan *pl, int node_tag, bool descend_into_subqueries);

// extract nodes with specific tag from an expression tree
List *ExtractNodesExpression(Node *node, int node_tag,
							 bool descend_into_subqueries);

// intermediate result type of given aggregate
Oid GetAggIntermediateResultType(Oid aggid);

// Identify the specific datatypes passed to an aggregate call.
int GetAggregateArgTypes(Aggref *aggref, Oid *inputTypes);

// Identify the transition state value's datatype for an aggregate call.
Oid ResolveAggregateTransType(Oid aggfnoid, Oid aggtranstype, Oid *inputTypes,
							  int numArguments);

// intermediate result of given aggregate
void GetAggregateInfo(Aggref *aggref, Oid *aggtransfn,
					  Oid *aggfinalfn, Oid *aggcombinefn, 
					  Oid *aggserialfn, Oid *aggdeserialfn,
					  Oid *aggtranstype, int *aggtransspace,
					  Datum *initValue, bool *initValueIsNull,
					  bool *shareable);

int
FindCompatibleAgg(List *agginfos, Aggref *newagg,
				  List **same_input_transnos);
int
FindCompatibleTrans(List *aggtransinfos, bool shareable,
					Oid aggtransfn, Oid aggtranstype,
					int transtypeLen, bool transtypeByVal,
					Oid aggcombinefn, Oid aggserialfn,
					Oid aggdeserialfn, Datum initValue, 
					bool initValueIsNull, List *transnos);


// replace Vars that reference JOIN outputs with references to the original
// relation variables instead
Query *FlattenJoinAliasVar(Query *query, gpos::ULONG query_level);

// is aggregate ordered
bool IsOrderedAgg(Oid aggid);

bool IsRepSafeAgg(Oid aggid);

// does aggregate have a combine function (and serial/deserial functions, if needed)
bool IsAggPartialCapable(Oid aggid);

// intermediate result type of given aggregate
Oid GetAggregate(const char *agg, Oid type_oid);

// array type oid
Oid GetArrayType(Oid typid);

// attribute stats slot
bool GetAttrStatsSlot(AttStatsSlot *sslot, HeapTuple statstuple, int reqkind,
					  Oid reqop, int flags);

// free attribute stats slot
void FreeAttrStatsSlot(AttStatsSlot *sslot);

// attribute statistics
HeapTuple GetAttStats(Oid relid, AttrNumber attnum);

List *GetExtStats(Relation rel);

char *GetExtStatsName(Oid statOid);
List *GetExtStatsKinds(Oid statOid);

// does a function exist with the given oid
bool FunctionExists(Oid oid);

// is the given function an allowed lossy cast for PS
bool IsFuncAllowedForPartitionSelection(Oid funcid);

// is the given function strict
bool FuncStrict(Oid funcid);

// does this preserve the NDVs of its inputs?
bool IsFuncNDVPreserving(Oid funcid);

// stability property of given function
char FuncStability(Oid funcid);

// exec location property of given function
char FuncExecLocation(Oid funcid);

// check constraint name
char *GetCheckConstraintName(Oid check_constraint_oid);

// check constraint relid
Oid GetCheckConstraintRelid(Oid check_constraint_oid);

// check constraint expression tree
Node *PnodeCheckConstraint(Oid check_constraint_oid);

// get the list of check constraints for a given relation
List *GetCheckConstraintOids(Oid rel_oid);

// part constraint expression tree
Node *GetRelationPartConstraints(Relation rel);

// get the cast function for the specified source and destination types
bool GetCastFunc(Oid src_oid, Oid dest_oid, bool *is_binary_coercible,
				 Oid *cast_fn_oid, CoercionPathType *pathtype);

// get type of operator
unsigned int GetComparisonType(Oid op_oid);

// get scalar comparison between given types
Oid GetComparisonOperator(Oid left_oid, Oid right_oid, unsigned int cmpt);

// get equality operator for given type
Oid GetEqualityOp(Oid type_oid);

// get equality operator for given ordering op (i.e. < or >)
Oid GetEqualityOpForOrderingOp(Oid opno, bool *reverse);

// get ordering operator for given equality op (i.e. =)
Oid GetOrderingOpForEqualityOp(Oid opno, bool *reverse);

// function name
char *GetFuncName(Oid funcid);

// output argument types of the given function
List *GetFuncOutputArgTypes(Oid funcid);

// process targetlist when function return type is record
List *ProcessRecordFuncTargetList(Oid funcid, List *targetList);

// argument types of the given function
List *GetFuncArgTypes(Oid funcid);

// does a function return a set of rows
bool GetFuncRetset(Oid funcid);

// return type of the given function
Oid GetFuncRetType(Oid funcid);

// commutator operator of the given operator
Oid GetCommutatorOp(Oid opno);

// inverse operator of the given operator
Oid GetInverseOp(Oid opno);

// function oid corresponding to the given operator oid
RegProcedure GetOpFunc(Oid opno);

// operator name
char *GetOpName(Oid opno);

// keys of the relation with the given oid
List *GetRelationKeys(Oid relid);

// relid of a composite type
Oid GetTypeRelid(Oid typid);

// name of the type with the given oid
char *GetTypeName(Oid typid);

// number of GP segments
int GetGPSegmentCount(void);

// heap attribute is null
bool HeapAttIsNull(HeapTuple tup, int attnum);

// free heap tuple
void FreeHeapTuple(HeapTuple htup);

// get the default hash opclass for type
Oid GetDefaultDistributionOpclassForType(Oid typid);

// get the column-definition hash opclass for type
Oid GetColumnDefOpclassForType(List *opclassName, Oid typid);

// get the default hash opfamily for type
Oid GetDefaultDistributionOpfamilyForType(Oid typid);
Oid GetDefaultPartitionOpfamilyForType(Oid typid);

// get the hash function in an opfamily for given datatype
Oid GetHashProcInOpfamily(Oid opfamily, Oid typid);

// is the given hash function a legacy cdbhash function?
Oid IsLegacyCdbHashFunction(Oid hashfunc);

// is the given hash function a legacy cdbhash function?
Oid GetLegacyCdbHashOpclassForBaseType(Oid typid);

// return the operator family the given opclass belongs to
Oid GetOpclassFamily(Oid opclass);

// append an element to a list
List *LAppend(List *list, void *datum);

// append an integer to a list
List *LAppendInt(List *list, int datum);

// append an oid to a list
List *LAppendOid(List *list, Oid datum);

// prepend a new element to the list
List *LPrepend(void *datum, List *list);

// prepend an integer to the list
List *LPrependInt(int datum, List *list);

// prepend an oid to a list
List *LPrependOid(Oid datum, List *list);

// concatenate lists
List *ListConcat(List *list1, List *list2);

// copy list
List *ListCopy(List *list);

// first cell in a list
ListCell *ListHead(List *l);

// last cell in a list
ListCell *ListTail(List *l);

// number of items in a list
uint32 ListLength(List *l);

// return the nth element in a list of pointers
void *ListNth(List *list, int n);

// return the nth element in a list of ints
int ListNthInt(List *list, int n);

// return the nth element in a list of oids
Oid ListNthOid(List *list, int n);

// check whether the given oid is a member of the given list
bool ListMemberOid(List *list, Oid oid);

// free list
void ListFree(List *list);

// deep free of a list
void ListFreeDeep(List *list);

// lookup type cache
TypeCacheEntry *LookupTypeCache(Oid type_id, int flags);

// create a value node for a string
Value *MakeStringValue(char *str);

// create a value node for an integer
Value *MakeIntegerValue(long i);

// create a constant of type int4
Node *MakeIntConst(int32 intValue);

// create a bool constant
Node *MakeBoolConst(bool value, bool isnull);

// make a NULL constant of the given type
Node *MakeNULLConst(Oid type_oid);

// make a NULL constant of the given type
Node *MakeSegmentFilterExpr(int segid);

// create a new target entry
TargetEntry *MakeTargetEntry(Expr *expr, AttrNumber resno, char *resname,
							 bool resjunk);

// create a new var node
Var *MakeVar(Index varno, AttrNumber varattno, Oid vartype, int32 vartypmod,
			 Index varlevelsup);

// memory allocation functions
void *MemCtxtAllocZeroAligned(MemoryContext context, Size size);
void *MemCtxtAllocZero(MemoryContext context, Size size);
void *MemCtxtRealloc(void *pointer, Size size);
void *GPDBAlloc(Size size);
void GPDBFree(void *ptr);

// create a duplicate of the given string in the given memory context
char *MemCtxtStrdup(MemoryContext context, const char *string);

// similar to ereport for logging messages
void GpdbEreportImpl(int xerrcode, int severitylevel, const char *xerrmsg,
					 const char *xerrhint, const char *filename, int lineno,
					 const char *funcname);
#define GpdbEreport(xerrcode, severitylevel, xerrmsg, xerrhint)       \
	gpdb::GpdbEreportImpl(xerrcode, severitylevel, xerrmsg, xerrhint, \
						  __FILE__, __LINE__, PG_FUNCNAME_MACRO)

// string representation of a node
char *NodeToString(void *obj);

// return the default value of the type
Node *GetTypeDefault(Oid typid);

// convert numeric to double; if out of range, return +/- HUGE_VAL
double NumericToDoubleNoOverflow(Numeric num);

// is the given Numeric value NaN?
bool NumericIsNan(Numeric num);

// convert time-related datums to double for stats purpose
double ConvertTimeValueToScalar(Datum datum, Oid typid);

// convert network-related datums to double for stats purpose
double ConvertNetworkToScalar(Datum datum, Oid typid);

// is the given operator hash-joinable
bool IsOpHashJoinable(Oid opno, Oid inputtype);

// is the given operator merge-joinable
bool IsOpMergeJoinable(Oid opno, Oid inputtype);

// is the given operator strict
bool IsOpStrict(Oid opno);

// does it preserve the NDVs of its inputs
bool IsOpNDVPreserving(Oid opno);

// get input types for a given operator
void GetOpInputTypes(Oid opno, Oid *lefttype, Oid *righttype);

// expression tree walker
bool WalkExpressionTree(Node *node, bool (*walker)(Node *,void *), void *context);

// query or expression tree walker
bool WalkQueryOrExpressionTree(Node *node, bool (*walker)(Node *, void *), void *context,
							   int flags);

// modify the components of a Query tree
Query *MutateQueryTree(Query *query, Node *(*mutator)(Node *, void *), void *context,
					   int flags);

bool WalkQueryTree(Query *query, bool (*walker)(Node *, void *), void *context,
				   int flags);

// modify an expression tree
Node *MutateExpressionTree(Node *node, Node *(*mutator)(Node *, void *), void *context);

// modify a query or an expression tree
Node *MutateQueryOrExpressionTree(Node *node, Node *(*mutator)(Node *, void *), void *context,
								  int flags);

// check whether a relation is inherited
bool HasSubclassSlow(Oid rel_oid);

// return the distribution policy of a relation; if the table is partitioned
// and the parts are distributed differently, return Random distribution
GpPolicy *GetDistributionPolicy(Relation rel);

// return true if the table is partitioned and hash-distributed, and one of
// the child partitions is randomly distributed
gpos::BOOL IsChildPartDistributionMismatched(Relation rel);

double CdbEstimatePartitionedNumTuples(Relation rel);

PageEstimate CdbEstimatePartitionedNumPages(Relation rel);

// close the given relation
void CloseRelation(Relation rel);

// return a list of index oids for a given relation
List *GetRelationIndexes(Relation relation);

// build an array of triggers for this relation
void BuildRelationTriggers(Relation rel);

PartitionKey GetRelationPartitionKey(Relation rel);

PartitionDesc RelationGetPartitionDesc(Relation rel, bool omit_detached);

MVNDistinct *GetMVNDistinct(Oid stat_oid);

MVDependencies *GetMVDependencies(Oid stat_oid);

// get relation with given oid
RelationWrapper GetRelation(Oid rel_oid);

// get ForeignScan node to scan a foreign table
ForeignScan *CreateForeignScan(Oid rel_oid, Index scanrelid, List *qual,
							   List *targetlist, Query *query,
							   RangeTblEntry *rte);

// return the first member of the given targetlist whose expression is
// equal to the given expression, or NULL if no such member exists
TargetEntry *FindFirstMatchingMemberInTargetList(Node *node, List *targetlist);

// return a list of members of the given targetlist whose expression is
// equal to the given expression, or NULL if no such member exists
List *FindMatchingMembersInTargetList(Node *node, List *targetlist);

// check if two gpdb objects are equal
bool Equals(void *p1, void *p2);

// check whether a type is composite
bool IsCompositeType(Oid typid);

bool IsTextRelatedType(Oid typid);

// create an empty 'StringInfoData' & return a pointer to it
StringInfo MakeStringInfo(void);

// append the two given strings to the StringInfo object
void AppendStringInfo(StringInfo str, const char *str1, const char *str2);

// look for the given node tags in the given tree and return the index of
// the first one found, or -1 if there are none
int FindNodes(Node *node, List *nodeTags);

// GPDB_91_MERGE_FIXME: collation
// look for nodes with non-default collation; returns 1 if any exist, -1 otherwise
int CheckCollation(Node *node);

Node *CoerceToCommonType(ParseState *pstate, Node *node, Oid target_type,
						 const char *context);

// replace any polymorphic type with correct data type deduced from input arguments
bool ResolvePolymorphicArgType(int numargs, Oid *argtypes, char *argmodes,
							   FuncExpr *call_expr);

// hash a list of const values with GPDB's hash function
int32 CdbHashConstList(List *constants, int num_segments, Oid *hashfuncs);

// get a random segment number
unsigned int CdbHashRandomSeg(int num_segments);

// check permissions on range table
void CheckRTPermissions(List *rtable);

// throw an error if table has update triggers.
bool HasUpdateTriggers(Oid relid);

// get index operator family properties
void IndexOpProperties(Oid opno, Oid opfamily, StrategyNumber *strategynumber,
					   Oid *righttype);

// check whether index column is returnable (for index-only scans)
gpos::BOOL IndexCanReturn(Relation index, int attno);

// get oids of families this operator belongs to
List *GetOpFamiliesForScOp(Oid opno);

// get the OID of hash equality operator(s) compatible with the given op
Oid GetCompatibleHashOpFamily(Oid opno);

// get the OID of legacy hash equality operator(s) compatible with the given op
Oid GetCompatibleLegacyHashOpFamily(Oid opno);

// get oids of op classes for the index keys
List *GetIndexOpFamilies(Oid index_oid);

// get oids of op classes for the merge join
List *GetMergeJoinOpFamilies(Oid opno);

// get the OID of base elementtype fora given typid
Oid GetBaseType(Oid typid);

// returns the result of evaluating 'expr' as an Expr. Caller keeps ownership of 'expr'
// and takes ownership of the result
Expr *EvaluateExpr(Expr *expr, Oid result_type, int32 typmod);

// extract string value from defelem's value
char *DefGetString(DefElem *defelem);

// transform array Const to an ArrayExpr
Expr *TransformArrayConstToArrayExpr(Const *constant);

// transform array Const to an ArrayExpr
Node *EvalConstExpressions(Node *node);

#ifdef FAULT_INJECTOR
// simple fault injector used by COptTasks.cpp to inject GPDB fault
FaultInjectorType_e InjectFaultInOptTasks(const char *fault_name);
#endif

// Does the metadata cache need to be reset (because of a catalog
// table has been changed?)
bool MDCacheNeedsReset(void);

// returns true if a query cancel is requested in GPDB
bool IsAbortRequested(void);

// Given the type OID, get the typelem (InvalidOid if not an array type).
Oid GetElementType(Oid array_type_oid);

GpPolicy *MakeGpPolicy(GpPolicyType ptype, int nattrs, int numsegments);


uint32 HashChar(Datum d);

uint32 HashBpChar(Datum d);

uint32 HashText(Datum d);

uint32 HashName(Datum d);

uint32 UUIDHash(Datum d);

void *GPDBMemoryContextAlloc(MemoryContext context, Size size);

MemoryContext GPDBAllocSetContextCreate();

void GPDBMemoryContextDelete(MemoryContext context);

List *GetRelChildIndexes(Oid reloid);

Oid GetForeignServerId(Oid reloid);

void GPDBLockRelationOid(Oid reloid, int lockmode);

char *GetRelFdwName(Oid reloid);

PathTarget *MakePathtargetFromTlist(List *tlist);

void SplitPathtargetAtSrfs(PlannerInfo *root, PathTarget *target,
						   PathTarget *input_target, List **targets,
						   List **targets_contain_srfs);

List *MakeTlistFromPathtarget(PathTarget *target);

Node *Expression_tree_mutator(Node *node, Node *(*mutator)(Node*, void*), void *context);

TargetEntry *TlistMember(Expr *node, List *targetlist);

Var *MakeVarFromTargetEntry(Index varno, TargetEntry *tle);

TargetEntry *FlatCopyTargetEntry(TargetEntry *src_tle);

bool IsTypeRange(Oid typid);

char *GetRelAmName(Oid reloid);

IndexAmRoutine *GetIndexAmRoutineFromAmHandler(Oid am_handler);

bool TestexprIsHashable(Node *testexpr, List *param_ids);

gpos::BOOL WalkQueryTree(Query *query, bool (*walker)(), void *context,
						 int flags);

}  //namespace gpdb

#define ForEach(cell, l) \
	for ((cell) = gpdb::ListHead(l); (cell) != NULL; (cell) = lnext(l, cell))

#define ForBoth(cell1, list1, cell2, list2)                                \
	for ((cell1) = gpdb::ListHead(list1), (cell2) = gpdb::ListHead(list2); \
		 (cell1) != NULL && (cell2) != NULL;                               \
		 (cell1) = lnext(list1, cell1), (cell2) = lnext(list2, cell2))

#define ForThree(cell1, list1, cell2, list2, cell3, list3)                 \
	for ((cell1) = gpdb::ListHead(list1), (cell2) = gpdb::ListHead(list2), \
		(cell3) = gpdb::ListHead(list3);                                   \
		 (cell1) != NULL && (cell2) != NULL && (cell3) != NULL;            \
		 (cell1) = lnext(list1, cell1), (cell2) = lnext(list2, cell2),                   \
		(cell3) = lnext(list3, cell3))

#define ForEachWithCount(cell, list, counter)                          \
	for ((cell) = gpdb::ListHead(list), (counter) = 0; (cell) != NULL; \
		 (cell) = lnext(list, cell), ++(counter))

#define ListMake1(x1) gpdb::LPrepend(x1, NIL)

#define ListMake2(x1, x2) gpdb::LPrepend(x1, ListMake1(x2))

#define ListMake1Int(x1) gpdb::LPrependInt(x1, NIL)

#define ListMake1Oid(x1) gpdb::LPrependOid(x1, NIL)
#define ListMake2Oid(x1, x2) gpdb::LPrependOid(x1, ListMake1Oid(x2))

#define LInitial(l) lfirst(gpdb::ListHead(l))

#define LInitialOID(l) lfirst_oid(gpdb::ListHead(l))

#define Palloc0Fast(sz)                                              \
	(MemSetTest(0, (sz))                                             \
		 ? gpdb::MemCtxtAllocZeroAligned(CurrentMemoryContext, (sz)) \
		 : gpdb::MemCtxtAllocZero(CurrentMemoryContext, (sz)))

#ifdef __GNUC__

/* With GCC, we can use a compound statement within an expression */
#define NewNode(size, tag)                                                \
	({                                                                    \
		Node *_result;                                                    \
		AssertMacro((size) >= sizeof(Node)); /* need the tag, at least */ \
		_result = (Node *) Palloc0Fast(size);                             \
		_result->type = (tag);                                            \
		_result;                                                          \
	})
#else

/*
 *	There is no way to dereference the palloc'ed pointer to assign the
 *	tag, and also return the pointer itself, so we need a holder variable.
 *	Fortunately, this macro isn't recursive so we just define
 *	a global variable for this purpose.
 */
extern PGDLLIMPORT Node *newNodeMacroHolder;

#define NewNode(size, tag)                                             \
	(AssertMacro((size) >= sizeof(Node)), /* need the tag, at least */ \
	 newNodeMacroHolder = (Node *) Palloc0Fast(size),                  \
	 newNodeMacroHolder->type = (tag), newNodeMacroHolder)
#endif	// __GNUC__

#define MakeNode(_type_) ((_type_ *) NewNode(sizeof(_type_), T_##_type_))

#define PStrDup(str) gpdb::MemCtxtStrdup(CurrentMemoryContext, (str))

#endif	// !GPDB_gpdbwrappers_H

// EOF
