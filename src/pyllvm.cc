#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>

#include <llvm/AsmParser/LLLexer.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/AsmParser/LLParser.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm-c/Core.h>
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"


namespace py = pybind11;
using namespace llvm;

#include "llvm/Support/TargetSelect.h"
#include "llvm/AsmParser/LLLexer.h"
#include "llvm/AsmParser/LLToken.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/AsmParser/SlotMapping.h"

template <class T> class ptr_wrapper
{
    public:
        ptr_wrapper() : ptr(nullptr) {}
        ptr_wrapper(T* ptr) : ptr(ptr) {}
        ptr_wrapper(const ptr_wrapper& other) : ptr(other.ptr) {}
        T& operator* () const { return *ptr; }
        T* operator->() const { return  ptr; }
        T* get() const { return ptr; }
        T& operator[](std::size_t idx) const { return ptr[idx]; }
    private:
        T* ptr;
};


    PYBIND11_DECLARE_HOLDER_TYPE(T, ptr_wrapper<T>, true);

PYBIND11_MODULE(pyllvm, m) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();


  llvm::lltok::Kind tok;


    
  auto lltok = m.def_submodule("lltok");
  
  #define VAL(x) lltok.attr(#x) = pybind11::int_((size_t)llvm::lltok::x);
  VAL(Eof)
  VAL(Error)
  VAL(dotdotdot)
  VAL(equal)
  VAL(comma)
  VAL(star)
  VAL(lsquare)
  VAL(rsquare)
  VAL(lbrace)
  VAL(rbrace)
  VAL(less)
  VAL(greater)
  VAL(lparen)
  VAL(rparen)
  VAL(exclaim)
  VAL(bar)
  VAL(colon)
  VAL(kw_vscale)
  VAL(kw_x)
  VAL(kw_true)
  VAL(kw_false)
  VAL(kw_declare)
  VAL(kw_define)
  VAL(kw_global)
  VAL(kw_constant)
  VAL(kw_dso_local)
  VAL(kw_dso_preemptable)
  VAL(kw_private)
  VAL(kw_internal)
  VAL(kw_linkonce)
  VAL(kw_linkonce_odr)
  VAL(kw_weak)
  VAL(kw_weak_odr)
  VAL(kw_appending)
  VAL(kw_dllimport)
  VAL(kw_dllexport)
  VAL(kw_common)
  VAL(kw_available_externally)
  VAL(kw_default)
  VAL(kw_hidden)
  VAL(kw_protected)
  VAL(kw_unnamed_addr)
  VAL(kw_local_unnamed_addr)
  VAL(kw_externally_initialized)
  VAL(kw_extern_weak)
  VAL(kw_external)
  VAL(kw_thread_local)
  VAL(kw_localdynamic)
  VAL(kw_initialexec)
  VAL(kw_localexec)
  VAL(kw_zeroinitializer)
  VAL(kw_undef)
  VAL(kw_null)
  VAL(kw_none)
  VAL(kw_to)
  VAL(kw_caller)
  VAL(kw_within)
  VAL(kw_from)
  VAL(kw_tail)
  VAL(kw_musttail)
  VAL(kw_notail)
  VAL(kw_target)
  VAL(kw_triple)
  VAL(kw_source_filename)
  VAL(kw_unwind)
  VAL(kw_deplibs)
  VAL(kw_datalayout)
  VAL(kw_volatile)
  VAL(kw_atomic)
  VAL(kw_unordered)
  VAL(kw_monotonic)
  VAL(kw_acquire)
  VAL(kw_release)
  VAL(kw_acq_rel)
  VAL(kw_seq_cst)
  VAL(kw_syncscope)
  VAL(kw_nnan)
  VAL(kw_ninf)
  VAL(kw_nsz)
  VAL(kw_arcp)
  VAL(kw_contract)
  VAL(kw_reassoc)
  VAL(kw_afn)
  VAL(kw_fast)
  VAL(kw_nuw)
  VAL(kw_nsw)
  VAL(kw_exact)
  VAL(kw_inbounds)
  VAL(kw_inrange)
  VAL(kw_align)
  VAL(kw_addrspace)
  VAL(kw_section)
  VAL(kw_partition)
  VAL(kw_alias)
  VAL(kw_ifunc)
  VAL(kw_module)
  VAL(kw_asm)
  VAL(kw_sideeffect)
  VAL(kw_alignstack)
  VAL(kw_inteldialect)
  VAL(kw_gc)
  VAL(kw_prefix)
  VAL(kw_prologue)
  VAL(kw_c)
  VAL(kw_cc)
  VAL(kw_ccc)
  VAL(kw_fastcc)
  VAL(kw_coldcc)
  VAL(kw_intel_ocl_bicc)
  VAL(kw_cfguard_checkcc)
  VAL(kw_x86_stdcallcc)
  VAL(kw_x86_fastcallcc)
  VAL(kw_x86_thiscallcc)
  VAL(kw_x86_vectorcallcc)
  VAL(kw_x86_regcallcc)
  VAL(kw_arm_apcscc)
  VAL(kw_arm_aapcscc)
  VAL(kw_arm_aapcs_vfpcc)
  VAL(kw_aarch64_vector_pcs)
  VAL(kw_aarch64_sve_vector_pcs)
  VAL(kw_msp430_intrcc)
  VAL(kw_avr_intrcc)
  VAL(kw_avr_signalcc)
  VAL(kw_ptx_kernel)
  VAL(kw_ptx_device)
  VAL(kw_spir_kernel)
  VAL(kw_spir_func)
  VAL(kw_x86_64_sysvcc)
  VAL(kw_win64cc)
  VAL(kw_webkit_jscc)
  VAL(kw_anyregcc)
  VAL(kw_swiftcc)
  VAL(kw_preserve_mostcc)
  VAL(kw_preserve_allcc)
  VAL(kw_ghccc)
  VAL(kw_x86_intrcc)
  VAL(kw_hhvmcc)
  VAL(kw_hhvm_ccc)
  VAL(kw_cxx_fast_tlscc)
  VAL(kw_amdgpu_vs)
  VAL(kw_amdgpu_ls)
  VAL(kw_amdgpu_hs)
  VAL(kw_amdgpu_es)
  VAL(kw_amdgpu_gs)
  VAL(kw_amdgpu_ps)
  VAL(kw_amdgpu_cs)
  VAL(kw_amdgpu_kernel)
  VAL(kw_tailcc)
  VAL(kw_attributes)
  VAL(kw_allocsize)
  VAL(kw_alwaysinline)
  VAL(kw_argmemonly)
  VAL(kw_sanitize_address)
  VAL(kw_sanitize_hwaddress)
  VAL(kw_sanitize_memtag)
  VAL(kw_builtin)
  VAL(kw_byval)
  VAL(kw_inalloca)
  VAL(kw_cold)
  VAL(kw_convergent)
  VAL(kw_dereferenceable)
  VAL(kw_dereferenceable_or_null)
  VAL(kw_inaccessiblememonly)
  VAL(kw_inaccessiblemem_or_argmemonly)
  VAL(kw_inlinehint)
  VAL(kw_inreg)
  VAL(kw_jumptable)
  VAL(kw_minsize)
  VAL(kw_naked)
  VAL(kw_nest)
  VAL(kw_noalias)
  VAL(kw_nobuiltin)
  VAL(kw_nocapture)
  VAL(kw_noduplicate)
  VAL(kw_nofree)
  VAL(kw_noimplicitfloat)
  VAL(kw_noinline)
  VAL(kw_norecurse)
  VAL(kw_nonlazybind)
  VAL(kw_nonnull)
  VAL(kw_noredzone)
  VAL(kw_noreturn)
  VAL(kw_nosync)
  VAL(kw_nocf_check)
  VAL(kw_nounwind)
  VAL(kw_optforfuzzing)
  VAL(kw_optnone)
  VAL(kw_optsize)
  VAL(kw_readnone)
  VAL(kw_readonly)
  VAL(kw_returned)
  VAL(kw_returns_twice)
  VAL(kw_signext)
  VAL(kw_speculatable)
  VAL(kw_ssp)
  VAL(kw_sspreq)
  VAL(kw_sspstrong)
  VAL(kw_safestack)
  VAL(kw_shadowcallstack)
  VAL(kw_sret)
  VAL(kw_sanitize_thread)
  VAL(kw_sanitize_memory)
  VAL(kw_speculative_load_hardening)
  VAL(kw_strictfp)
  VAL(kw_swifterror)
  VAL(kw_swiftself)
  VAL(kw_uwtable)
  VAL(kw_willreturn)
  VAL(kw_writeonly)
  VAL(kw_zeroext)
  VAL(kw_immarg)
  VAL(kw_type)
  VAL(kw_opaque)
  VAL(kw_comdat)
  VAL(kw_any)
  VAL(kw_exactmatch)
  VAL(kw_largest)
  VAL(kw_noduplicates)
  VAL(kw_samesize)
  VAL(kw_eq)
  VAL(kw_ne)
  VAL(kw_slt)
  VAL(kw_sgt)
  VAL(kw_sle)
  VAL(kw_sge)
  VAL(kw_ult)
  VAL(kw_ugt)
  VAL(kw_ule)
  VAL(kw_uge)
  VAL(kw_oeq)
  VAL(kw_one)
  VAL(kw_olt)
  VAL(kw_ogt)
  VAL(kw_ole)
  VAL(kw_oge)
  VAL(kw_ord)
  VAL(kw_uno)
  VAL(kw_ueq)
  VAL(kw_une)
  VAL(kw_xchg)
  VAL(kw_nand)
  VAL(kw_max)
  VAL(kw_min)
  VAL(kw_umax)
  VAL(kw_umin)
  VAL(kw_fneg)
  VAL(kw_add)
  VAL(kw_fadd)
  VAL(kw_sub)
  VAL(kw_fsub)
  VAL(kw_mul)
  VAL(kw_fmul)
  VAL(kw_udiv)
  VAL(kw_sdiv)
  VAL(kw_fdiv)
  VAL(kw_urem)
  VAL(kw_srem)
  VAL(kw_frem)
  VAL(kw_shl)
  VAL(kw_lshr)
  VAL(kw_ashr)
  VAL(kw_and)
  VAL(kw_or)
  VAL(kw_xor)
  VAL(kw_icmp)
  VAL(kw_fcmp)
  VAL(kw_phi)
  VAL(kw_call)
  VAL(kw_trunc)
  VAL(kw_zext)
  VAL(kw_sext)
  VAL(kw_fptrunc)
  VAL(kw_fpext)
  VAL(kw_uitofp)
  VAL(kw_sitofp)
  VAL(kw_fptoui)
  VAL(kw_fptosi)
  VAL(kw_inttoptr)
  VAL(kw_ptrtoint)
  VAL(kw_bitcast)
  VAL(kw_addrspacecast)
  VAL(kw_select)
  VAL(kw_va_arg)
  VAL(kw_landingpad)
  VAL(kw_personality)
  VAL(kw_cleanup)
  VAL(kw_catch)
  VAL(kw_filter)
  VAL(kw_ret)
  VAL(kw_br)
  VAL(kw_switch)
  VAL(kw_indirectbr)
  VAL(kw_invoke)
  VAL(kw_resume)
  VAL(kw_unreachable)
  VAL(kw_cleanupret)
  VAL(kw_catchswitch)
  VAL(kw_catchret)
  VAL(kw_catchpad)
  VAL(kw_cleanuppad)
  VAL(kw_callbr)
  VAL(kw_alloca)
  VAL(kw_load)
  VAL(kw_store)
  VAL(kw_fence)
  VAL(kw_cmpxchg)
  VAL(kw_atomicrmw)
  VAL(kw_getelementptr)
  VAL(kw_extractelement)
  VAL(kw_insertelement)
  VAL(kw_shufflevector)
  VAL(kw_extractvalue)
  VAL(kw_insertvalue)
  VAL(kw_blockaddress)
  VAL(kw_freeze)
  VAL(kw_distinct)
  VAL(kw_uselistorder)
  VAL(kw_uselistorder_bb)
  VAL(kw_path)
  VAL(kw_hash)
  VAL(kw_gv)
  VAL(kw_guid)
  VAL(kw_name)
  VAL(kw_summaries)
  VAL(kw_flags)
  VAL(kw_linkage)
  VAL(kw_notEligibleToImport)
  VAL(kw_live)
  VAL(kw_dsoLocal)
  VAL(kw_canAutoHide)
  VAL(kw_function)
  VAL(kw_insts)
  VAL(kw_funcFlags)
  VAL(kw_readNone)
  VAL(kw_readOnly)
  VAL(kw_noRecurse)
  VAL(kw_returnDoesNotAlias)
  VAL(kw_noInline)
  VAL(kw_alwaysInline)
  VAL(kw_calls)
  VAL(kw_callee)
  VAL(kw_hotness)
  VAL(kw_unknown)
  VAL(kw_hot)
  VAL(kw_critical)
  VAL(kw_relbf)
  VAL(kw_variable)
  VAL(kw_vTableFuncs)
  VAL(kw_virtFunc)
  VAL(kw_aliasee)
  VAL(kw_refs)
  VAL(kw_typeIdInfo)
  VAL(kw_typeTests)
  VAL(kw_typeTestAssumeVCalls)
  VAL(kw_typeCheckedLoadVCalls)
  VAL(kw_typeTestAssumeConstVCalls)
  VAL(kw_typeCheckedLoadConstVCalls)
  VAL(kw_vFuncId)
  VAL(kw_offset)
  VAL(kw_args)
  VAL(kw_typeid)
  VAL(kw_typeidCompatibleVTable)
  VAL(kw_summary)
  VAL(kw_typeTestRes)
  VAL(kw_kind)
  VAL(kw_unsat)
  VAL(kw_byteArray)
  VAL(kw_inline)
  VAL(kw_single)
  VAL(kw_allOnes)
  VAL(kw_sizeM1BitWidth)
  VAL(kw_alignLog2)
  VAL(kw_sizeM1)
  VAL(kw_bitMask)
  VAL(kw_inlineBits)
  VAL(kw_wpdResolutions)
  VAL(kw_wpdRes)
  VAL(kw_indir)
  VAL(kw_singleImpl)
  VAL(kw_branchFunnel)
  VAL(kw_singleImplName)
  VAL(kw_resByArg)
  VAL(kw_byArg)
  VAL(kw_uniformRetVal)
  VAL(kw_uniqueRetVal)
  VAL(kw_virtualConstProp)
  VAL(kw_info)
  VAL(kw_byte)
  VAL(kw_bit)
  VAL(kw_varFlags)
  VAL(LabelID)
  VAL(GlobalID)
  VAL(LocalVarID)
  VAL(AttrGrpID)
  VAL(SummaryID)
  VAL(LabelStr)
  VAL(GlobalVar)
  VAL(ComdatVar)
  VAL(LocalVar)
  VAL(MetadataVar)
  VAL(StringConstant)
  VAL(DwarfTag)
  VAL(DwarfAttEncoding)
  VAL(DwarfVirtuality)
  VAL(DwarfLang)
  VAL(DwarfCC)
  VAL(EmissionKind)
  VAL(NameTableKind)
  VAL(DwarfOp)
  VAL(DIFlag)
  VAL(DISPFlag)
  VAL(DwarfMacinfo)
  VAL(ChecksumKind)
  VAL(Type)
  VAL(APFloat)
  VAL(APSInt)
    
  py::class_<llvm::LLLexer, std::shared_ptr<llvm::LLLexer>>(m, "LLLexer")
    .def("getTok", [](std::shared_ptr<llvm::LLLexer> lm) {
        try {
            const char* str1 = lm->getLoc().getPointer();
            auto toktype = (size_t)lm->Lex(); 
            const char* str2 = lm->getLoc().getPointer(); 
            // llvm::errs() << (void*) str1 << " " << (void*) str2 << " " << toktype << "\n";

            std::string tok; 
            if(str2 >= str1){
                if(str1== nullptr){
                    llvm::errs() << "PROBLEM1111 " << toktype << "\n"; 
                    return std::make_pair(toktype,std::string("")); 
                }else{ 
                    tok = std::string(str1,str2-str1); 
                }
            }else{ 
                llvm::errs() << "PROBLEM2 " << toktype << "\n"; 
                return std::make_pair(toktype,std::string("")); 
            }
            return std::make_pair(toktype,tok);
        }catch(std::exception const& e){
            llvm::errs() << "PROBLEM3" << "\n"; 
            std::cout << "Exception: " << e.what() << "\n";
        }
       
    })

    .def("getFirstTok", [](std::shared_ptr<llvm::LLLexer> lm) {
        auto toktype = (size_t)lm->Lex(); 
        return std::make_pair(toktype,std::string(""));
    })
    .def("getTokType", [](std::shared_ptr<llvm::LLLexer> lm) {
        return (size_t)lm->Lex();
    })
    .def("getTokStr", [](std::shared_ptr<llvm::LLLexer> lm) {
        const char* str1 = lm->getLoc().getPointer();
        lm->Lex(); 
        const char* str2 = lm->getLoc().getPointer();
        std::string result(str1,str2-str1);
        return result;
    })
    .def("getStrVal", [](std::shared_ptr<llvm::LLLexer> lm) {
        return lm->getStrVal();
    })
    .def("getTypeVal", [](std::shared_ptr<llvm::LLLexer> lm) {
        return ptr_wrapper<llvm::Type>(lm->getTyVal());
    }, py::return_value_policy::reference)
    .def("getUIntVal", [](std::shared_ptr<llvm::LLLexer> lm) {
        return lm->getUIntVal();
    })
    .def("getAPSIntVal", [](std::shared_ptr<llvm::LLLexer> lm) {
        return lm->getAPSIntVal().toString(10);
    })
    .def("getAPFloatVal", [](std::shared_ptr<llvm::LLLexer> lm) {
        llvm::SmallVector<char, 10> chars;
        lm->getAPFloatVal().toString(chars);
        return std::string(chars.data(), chars.size());
    });
    
    
  py::class_<llvm::Type, ptr_wrapper<llvm::Type>>(m, "LLVMType")
     #define NEX(a) .def(#a, [=](ptr_wrapper<llvm::Type> ty) { return ty->a(); })
     NEX(isVoidTy)
      NEX(isHalfTy)
     NEX(isFloatTy)
      NEX(isDoubleTy)
      NEX(isX86_FP80Ty)
      NEX(isFP128Ty )
      NEX(isPPC_FP128Ty)
      NEX(isX86_MMXTy )
      NEX(isLabelTy)
      NEX(isTokenTy)
      NEX(isIntegerTy)      
    .def("getIntWidth", [=](ptr_wrapper<llvm::Type> ty) {
        return cast<IntegerType>(ty.get())->getBitWidth();
    })
      ;

llvm::SourceMgr *sm = new SourceMgr();
llvm::SMDiagnostic *sd = new SMDiagnostic();
  m.def("lexer", [&](std::string &s) {
      llvm::LLVMContext &Ctx = *unwrap(LLVMGetGlobalContext());
      //obvious memory leak, but need to do something with memory since not taken
      return std::shared_ptr<llvm::LLLexer>(new llvm::LLLexer(*(new std::string(s)),  *sm, *sd, Ctx));
  });
//   m.def("printErrors", [&]() {
//       sd->print("lexer", llvm::outs(), false);
//   });


//   m.def("parseArgList", [&](std::string &s)  {
//       llvm::StringRef str = StringRef(s);
//       llvm::SourceMgr smSourceMgr; 
//       llvm::SMDiagnostic sd;
//       llvm::LLVMContext &TheContext = *unwrap(LLVMGetGlobalContext());
//       llvm::Module M("parse type", TheContext); 
//       //const llvm::SlotMapping *Slots; 
//       ModuleSummaryIndex MSI(true);
//       llvm::errs() << str << "\n"; 
//       LLParser ll(str, smSourceMgr, sd, &M, &MSI, TheContext);
//       SmallVector<LLParser::ArgInfo, 4>args;
//       bool isVarArg = false;
//       std::vector<llvm::Type*> types;
//       for(auto info : args){
//           types.push_back(info.Ty);
//       }
//       sd.print("carl",llvm::errs());
//       return std::make_pair(types, isVarArg);
//   });
//   m.def("parseType", [&](std::string &s) -> std::string {
//       llvm::StringRef str = StringRef(s);
//       llvm::LLVMContext &TheContext = *unwrap(LLVMGetGlobalContext());
//       std::unique_ptr<llvm::Module> M = std::make_unique<Module>("parse type", TheContext); 
//       //const llvm::SlotMapping *Slots; 
//       unsigned Read = 0; 
//       if (llvm::parseTypeAtBeginning(str, Read,  *sd, *M))
//         return s.substr(0, Read);
//        else return "";
//   });

}
