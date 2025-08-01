//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2010 Greenplum, Inc.
//
//	@filename:
//		CTranslatorDXLToPlStmt.h
//
//	@doc:
//		Class providing methods for translating from DXL tree to GPDB PlannedStmt
//
//	@test:
//
//
//---------------------------------------------------------------------------

#ifndef GPDXL_CTranslatorDxlToPlStmt_H
#define GPDXL_CTranslatorDxlToPlStmt_H

extern "C" {
#include "postgres.h"
}

#include "gpos/base.h"

#include "gpopt/translate/CContextDXLToPlStmt.h"
#include "gpopt/translate/CDXLTranslateContext.h"
#include "gpopt/translate/CDXLTranslateContextBaseTable.h"
#include "gpopt/translate/CMappingColIdVarPlStmt.h"
#include "gpopt/translate/CTranslatorDXLToScalar.h"
#include "naucrates/dxl/CIdGenerator.h"
#include "naucrates/dxl/operators/CDXLCtasStorageOptions.h"
#include "naucrates/dxl/operators/CDXLPhysicalIndexScan.h"

#include "access/attnum.h"
#include "nodes/nodes.h"
#include "nodes/plannodes.h"

// fwd declarations
namespace gpopt
{
class CMDAccessor;
}

namespace gpmd
{
class IMDRelation;
class IMDIndex;
}  // namespace gpmd

struct PlannedStmt;
struct Scan;
struct HashJoin;
struct NestLoop;
struct MergeJoin;
struct Hash;
struct RangeTblEntry;
struct Motion;
struct Limit;
struct Agg;
struct Append;
struct Sort;
struct SubqueryScan;
struct SubPlan;
struct Result;
struct Material;
struct ShareInputScan;
//struct Const;
//struct List;

namespace gpdxl
{
using namespace gpopt;

// fwd decl
class CDXLNode;
class CDXLPhysicalCTAS;
class CDXLDirectDispatchInfo;

//---------------------------------------------------------------------------
//	@class:
//		CTranslatorDXLToPlStmt
//
//	@doc:
//		Class providing methods for translating from DXL tree to GPDB PlannedStmt
//
//---------------------------------------------------------------------------
class CTranslatorDXLToPlStmt
{
private:
	// context for fixing index var attno
	struct SContextIndexVarAttno
	{
		// MD relation
		const IMDRelation *m_md_rel;

		// MD index
		const IMDIndex *m_md_index;

		// ctor
		SContextIndexVarAttno(const IMDRelation *md_rel,
							  const IMDIndex *md_index)
			: m_md_rel(md_rel), m_md_index(md_index)
		{
			GPOS_ASSERT(nullptr != md_rel);
			GPOS_ASSERT(nullptr != md_index);
		}
	};	// SContextIndexVarAttno

	// context for finding security quals in an RTE
	struct SContextSecurityQuals
	{
		// relid of the RTE to search in the rewritten parse tree
		const OID m_relId;

		// List to hold the security quals present in an RTE
		List *m_security_quals{NIL};

		// ctor
		SContextSecurityQuals(const OID relId) : m_relId(relId)
		{
		}
	};	// SContextSecurityQuals

	// memory pool
	CMemoryPool *m_mp;

	// meta data accessor
	CMDAccessor *m_md_accessor;

	CContextDXLToPlStmt *m_dxl_to_plstmt_context;

	CTranslatorDXLToScalar *m_translator_dxl_to_scalar;

	// command type
	CmdType m_cmd_type;

	// is target table distributed, false when in non DML statements
	BOOL m_is_tgt_tbl_distributed;

	// list of result relations range table indexes for DML statements,
	// or NULL for select queries
	List *m_result_rel_list;

	// number of segments
	ULONG m_num_of_segments;

	// partition selector counter
	ULONG m_partition_selector_counter;

	// private copy ctor
	CTranslatorDXLToPlStmt(const CTranslatorDXLToPlStmt &);

	// walker to set index var attno's
	static BOOL SetIndexVarAttnoWalker(
		Node *node, SContextIndexVarAttno *ctxt_index_var_attno_walker);

	// walker to set inner var to outer
	static BOOL SetHashKeysVarnoWalker(Node *node, void *context);

	static BOOL FetchSecurityQualsWalker(
		Node *node, SContextSecurityQuals *ctxt_security_quals);

	static BOOL FetchSecurityQuals(Query *parsetree,
								   SContextSecurityQuals *ctxt_security_quals);

	static BOOL SetSecurityQualsVarnoWalker(Node *node, Index *index);

public:
	// ctor
	CTranslatorDXLToPlStmt(CMemoryPool *mp, CMDAccessor *md_accessor,
						   CContextDXLToPlStmt *dxl_to_plstmt_context,
						   ULONG num_of_segments);

	// dtor
	~CTranslatorDXLToPlStmt();

	// translate DXL operator node into a Plan node
	Plan *TranslateDXLOperatorToPlan(
		const CDXLNode *dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// main translation routine for DXL tree -> PlannedStmt
	PlannedStmt *GetPlannedStmtFromDXL(const CDXLNode *dxlnode,
									   const Query *orig_query,
									   bool can_set_tag);

	// translate the join types from its DXL representation to the GPDB one
	static JoinType GetGPDBJoinTypeFromDXLJoinType(EdxlJoinType join_type);

private:
	// Set the bitmapset of a plan to the list of param_ids defined by the plan
	static void SetParamIds(Plan *);

	static List *TranslatePartOids(IMdIdArray *parts, INT lockmode);

	static List *TranslateJoinPruneParamids(
		const ULongPtrArray *selector_ids, OID oid_type,
		CContextDXLToPlStmt *dxl_to_plstmt_context);

	void TranslatePlan(
		Plan *plan, const CDXLNode *dxlnode,
		CDXLTranslateContext *output_context,
		CContextDXLToPlStmt *dxl_to_plstmt_context,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings);

	// translate DXL table scan node into a SeqScan node
	Plan *TranslateDXLTblScan(
		const CDXLNode *tbl_scan_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL index scan node into a IndexScan node
	Plan *TranslateDXLIndexScan(
		const CDXLNode *index_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translates a DXL index scan node into a IndexScan node
	Plan *TranslateDXLIndexScan(
		const CDXLNode *index_scan_dxlnode,
		gpdxl::CDXLPhysicalIndexScan *dxl_physical_idx_scan_op,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL index scan node into a IndexOnlyScan node
	Plan *TranslateDXLIndexOnlyScan(
		const CDXLNode *index_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL hash join into a HashJoin node
	Plan *TranslateDXLHashJoin(
		const CDXLNode *TranslateDXLHashJoin,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL nested loop join into a NestLoop node
	Plan *TranslateDXLNLJoin(
		const CDXLNode *nl_join_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL merge join into a MergeJoin node
	Plan *TranslateDXLMergeJoin(
		const CDXLNode *merge_join_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL motion node into GPDB Motion plan node
	Plan *TranslateDXLMotion(
		const CDXLNode *motion_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL motion node
	Plan *TranslateDXLDuplicateSensitiveMotion(
		const CDXLNode *motion_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL duplicate sensitive redistribute motion node into
	// GPDB result node with hash filters
	Plan *TranslateDXLRedistributeMotionToResultHashFilters(
		const CDXLNode *motion_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL aggregate node into GPDB Agg plan node
	Plan *TranslateDXLAgg(
		const CDXLNode *motion_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL window node into GPDB window node
	Plan *TranslateDXLWindow(
		const CDXLNode *motion_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL sort node into GPDB Sort plan node
	Plan *TranslateDXLSort(
		const CDXLNode *sort_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a DXL node into a Hash node
	Plan *TranslateDXLHash(
		const CDXLNode *dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL Limit node into a Limit node
	Plan *TranslateDXLLimit(
		const CDXLNode *limit_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate DXL TVF into a GPDB Function Scan node
	Plan *TranslateDXLTvf(
		const CDXLNode *tvf_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);


	Plan *TranslateDXLProjectSet(const CDXLNode *result_dxlnode);

	Plan *CreateProjectSetNodeTree(const CDXLNode *result_dxlnode,
								   Plan *result_node_plan, Plan *child_plan,
								   Plan *&project_set_child_plan,
								   BOOL &will_require_result_node);

	void MutateFuncExprToVarProjectSet(Plan *final_plan);

	Plan *TranslateDXLResult(
		const CDXLNode *result_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	Plan *TranslateDXLAppend(
		const CDXLNode *append_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	Plan *TranslateDXLMaterialize(
		const CDXLNode *materialize_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	Plan *TranslateDXLSharedScan(
		const CDXLNode *shared_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a sequence operator
	Plan *TranslateDXLSequence(
		const CDXLNode *sequence_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a dynamic table scan operator
	Plan *TranslateDXLDynTblScan(
		const CDXLNode *dyn_tbl_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a dynamic index scan operator
	Plan *TranslateDXLDynIdxScan(
		const CDXLNode *dyn_idx_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a dynamic index only scan operator
	Plan *TranslateDXLDynIdxOnlyScan(
		const CDXLNode *dyn_idx_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a dynamic foreign scan operator
	Plan *TranslateDXLDynForeignScan(
		const CDXLNode *dyn_foreign_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a DML operator
	Plan *TranslateDXLDml(
		const CDXLNode *dml_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a Split operator
	Plan *TranslateDXLSplit(
		const CDXLNode *split_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate an Assert operator
	Plan *TranslateDXLAssert(
		const CDXLNode *assert_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a CTE producer into a GPDB share input scan
	Plan *TranslateDXLCTEProducerToSharedScan(
		const CDXLNode *cte_producer_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a CTE consumer into a GPDB share input scan
	Plan *TranslateDXLCTEConsumerToSharedScan(
		const CDXLNode *cte_consumer_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a (dynamic) bitmap table scan operator
	Plan *TranslateDXLBitmapTblScan(
		const CDXLNode *bitmapscan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a DXL PartitionSelector into a GPDB PartitionSelector
	Plan *TranslateDXLPartSelector(
		const CDXLNode *partition_selector_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *
			ctxt_translation_prev_siblings	// translation contexts of previous siblings
	);

	// translate a DXL Value Scan into GPDB Value Scan
	Plan *TranslateDXLValueScan(
		const CDXLNode *value_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings);

	// translate DXL filter list into GPDB filter list
	List *TranslateDXLFilterList(
		const CDXLNode *filter_list_dxlnode,
		const CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *child_contexts,
		CDXLTranslateContext *output_context);

	// create range table entry from a CDXLPhysicalTVF node
	RangeTblEntry *TranslateDXLTvfToRangeTblEntry(
		const CDXLNode *tvf_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslateContextBaseTable *base_table_context);

	// create range table entry from a CDXLPhysicalValueScan node
	RangeTblEntry *TranslateDXLValueScanToRangeTblEntry(
		const CDXLNode *value_scan_dxlnode,
		CDXLTranslateContext *output_context,
		CDXLTranslateContextBaseTable *base_table_context);

	// create range table entry from a table descriptor
	Index ProcessDXLTblDescr(const CDXLTableDescr *table_descr,
							 CDXLTranslateContextBaseTable *base_table_context);

	// translate DXL projection list into a target list
	List *TranslateDXLProjList(
		const CDXLNode *project_list_dxlnode,
		const CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *child_contexts,
		CDXLTranslateContext *output_context);

	// insert NULL values for dropped attributes to construct the target list for a DML statement
	List *CreateTargetListWithNullsForDroppedCols(List *target_list,
												  const IMDRelation *md_rel,
												  bool keepDropedAsNull);

	// create a target list containing column references for a hash node from the
	// project list of its child node
	static List *TranslateDXLProjectListToHashTargetList(
		const CDXLNode *project_list_dxlnode,
		CDXLTranslateContext *child_context,
		CDXLTranslateContext *output_context);

	List *TranslateDXLFilterToQual(
		const CDXLNode *filter_dxlnode,
		const CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *child_contexts,
		CDXLTranslateContext *output_context);


	// translate operator costs from the DXL cost structure into a Plan
	// struct used by GPDB
	void TranslatePlanCosts(const CDXLNode *dxlnode, Plan *plan);

	// shortcut for translating both the projection list and the filter
	void TranslateProjListAndFilter(
		const CDXLNode *project_list_dxlnode, const CDXLNode *filter_dxlnode,
		const CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *child_contexts, List **targetlist_out,
		List **qual_out, CDXLTranslateContext *output_context);

	void AddSecurityQuals(OID relId, List **qual, Index *index);

	// translate the hash expr list of a redistribute motion node
	void TranslateHashExprList(const CDXLNode *hash_expr_list_dxlnode,
							   const CDXLTranslateContext *child_context,
							   List **hash_expr_out_list,
							   List **hash_expr_types_out_list,
							   CDXLTranslateContext *output_context);

	// translate the tree of bitmap index operators that are under a (dynamic) bitmap table scan
	Plan *TranslateDXLBitmapAccessPath(
		const CDXLNode *bitmap_access_path_dxlnode,
		CDXLTranslateContext *output_context, const IMDRelation *md_rel,
		const CDXLTableDescr *table_descr,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings,
		BitmapHeapScan *bitmap_tbl_scan);

	// translate a bitmap bool op expression
	Plan *TranslateDXLBitmapBoolOp(
		const CDXLNode *bitmap_boolop_dxlnode,
		CDXLTranslateContext *output_context, const IMDRelation *md_rel,
		const CDXLTableDescr *table_descr,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings,
		BitmapHeapScan *bitmap_tbl_scan);

	// translate CDXLScalarBitmapIndexProbe into BitmapIndexScan or DynamicBitmapIndexScan
	Plan *TranslateDXLBitmapIndexProbe(
		const CDXLNode *bitmap_index_probe_dxlnode,
		CDXLTranslateContext *output_context, const IMDRelation *md_rel,
		const CDXLTableDescr *table_descr,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings,
		BitmapHeapScan *bitmap_tbl_scan);

	static void TranslateSortCols(const CDXLNode *sort_col_list_dxl,
								  const CDXLTranslateContext *child_context,
								  AttrNumber *att_no_sort_colids,
								  Oid *sort_op_oids, Oid *sort_collations_oids,
								  bool *is_nulls_first);

	List *TranslateDXLScCondToQual(
		const CDXLNode *filter_dxlnode,
		const CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *child_contexts,
		CDXLTranslateContext *output_context);

	// parse string value into a Const
	static Cost CostFromStr(const CWStringBase *str);

	// check if the given operator is a DML operator on a distributed table
	BOOL IsTgtTblDistributed(CDXLOperator *dxlop);

	// add a target entry for a junk column with given colid to the target list
	static void AddJunkTargetEntryForColId(
		List **target_list, CDXLTranslateContext *dxl_translate_ctxt,
		ULONG colid, const char *resname);

	// translate the index condition list in an Index scan
	void TranslateIndexConditions(
		CDXLNode *index_cond_list_dxlnode, const CDXLTableDescr *dxl_tbl_descr,
		BOOL is_bitmap_index_probe, const IMDIndex *index,
		const IMDRelation *md_rel, CDXLTranslateContext *output_context,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings,
		List **index_cond, List **index_orig_cond);

	// translate the index filters
	List *TranslateDXLIndexFilter(
		CDXLNode *filter_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings);

	// translate the assert constraints
	List *TranslateDXLAssertConstraints(
		CDXLNode *filter_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *child_contexts);

	// translate a CTAS operator
	Plan *TranslateDXLCtas(
		const CDXLNode *dml_dxlnode, CDXLTranslateContext *output_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings =
			nullptr	 // translation contexts of previous siblings
	);

	// sets the vartypmod fields in the target entries of the given target list
	static void SetVarTypMod(const CDXLPhysicalCTAS *dxlop, List *target_list);

	// translate the distribution policy for a DXL physical CTAS operator
	static GpPolicy *TranslateDXLPhyCtasToDistrPolicy(
		const CDXLPhysicalCTAS *dxlop, List *target_list);

	// translate CTAS storage options
	static List *TranslateDXLCtasStorageOptions(
		gpdxl::CDXLCtasStorageOptions::CDXLCtasOptionArray
			*ctas_storage_options);

	// compute directed dispatch segment ids
	List *TranslateDXLDirectDispatchInfo(
		CDXLDirectDispatchInfo *dxl_direct_dispatch_info,
		RangeTblEntry *pRTEHashFuncCal);

	// hash a DXL datum with GPDB's hash function
	ULONG GetDXLDatumGPDBHash(CDXLDatumArray *dxl_datum_array,
							  RangeTblEntry *pRTEHashFuncCal);

	// translate nest loop colrefs to GPDB nestparams
	static List *TranslateNestLoopParamList(
		CDXLColRefArray *pdrgdxlcrOuterRefs, CDXLTranslateContext *dxltrctxLeft,
		CDXLTranslateContext *dxltrctxRight);

	// check if the target list contains any system columns
	static void CheckSafeTargetListForAOTables(List *target_list);

	// create final target list for update
	static List *CreateDirectCopyTargetList(List *target_list);

	// get all non-dropped columns of a relation
	static List *GetRelationActiveColums(const IMDRelation *md_rel);
	
	static Node *FixUpperExprMutatorProjectSet(Node *node, void *context);

	// checks if index is used for Order by.
	bool IsIndexForOrderBy(
		CDXLTranslateContextBaseTable *base_table_context,
		CDXLTranslationContextArray *ctxt_translation_prev_siblings,
		CDXLTranslateContext *output_context,
		CDXLNode *index_cond_list_dxlnode);

	// fill the aggno and transno for the aggnode
	static void TranslateAggFillInfo(CContextDXLToPlStmt *context, Aggref *aggref);
};
}  // namespace gpdxl

#endif	// !GPDXL_CTranslatorDxlToPlStmt_H

// EOF
