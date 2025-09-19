#pragma once

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#    pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif
#if defined(__GNUC__) && defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunused-parameter"
#    pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/Basic/SourceManager.h>

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic pop
#endif
#if defined(__GNUC__) && defined(__clang__)
#    pragma GCC diagnostic pop
#endif

#include <unordered_set>
#include <utils/naming.hpp>
#include <xcml_func.hpp>
#include <xcml_type.hpp>
#include <xcml_utils.hpp>
#include "error_trace.hpp"
#include "transform_info.hpp"
#include "utils.hpp"
#include "visitor.hpp"

namespace u = xcml::utils;

struct visitor_base {
    explicit visitor_base(transform_info& info, xcml::compound_stmt_ptr const& scope,
                          clang::Expr const* current_kernel,
                          clang::FunctionDecl const* current_func, bool current_is_kernel)
        : info_(info),
          ast_(info.ctx()),
          scope_(scope),
          current_kernel_(current_kernel),
          current_func_(current_func),
          current_is_kernel_(current_is_kernel) {}

    void visit_decl(clang::Decl const* decl, context const& = context()) {
        visit_decl(scope_, decl);
    }

    void visit_decl(xcml::compound_stmt_ptr const& scope, clang::Decl const* decl,
                    context const& = context());

    void visit_stmt(clang::Stmt const* stmt, context const& = context()) {
        visit_stmt(scope_, stmt);
    }

    void visit_stmt(xcml::compound_stmt_ptr const& scope, clang::Stmt const* stmt,
                    context const& = context());

    void descend_compound_stmt(xcml::compound_stmt_ptr const& scope,
                               clang::CompoundStmt const* stmt, xcml::expr_ptr* last_value,
                               context const& = context()) {
        for (auto stmt : stmt->body()) {
            if (auto decl = clang::dyn_cast<clang::DeclStmt>(stmt)) {
                PUSH_CONTEXT(decl);

                add_to_scope(scope, decl);
            } else if (auto const* expr = clang::dyn_cast<clang::Expr>(stmt)) {
                PUSH_CONTEXT(expr);

                auto val = visit_expr(scope, expr);
                if (last_value) {
                    *last_value = val;
                }
            } else {
                PUSH_CONTEXT(stmt);

                visit_stmt(scope, stmt);
            }
        }
    }

    xcml::compound_stmt_ptr visit_compound_stmt(xcml::compound_stmt_ptr const& scope,
                                                clang::CompoundStmt const* stmt,
                                                clang::FunctionDecl const* function,
                                                bool function_is_kernel,
                                                xcml::expr_ptr* last_value,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(clang::CompoundStmt const* stmt,
                                                clang::FunctionDecl const* function,
                                                bool function_is_kernel,
                                                xcml::expr_ptr* last_value = nullptr,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(clang::Stmt const* stmt,
                                                clang::FunctionDecl const* function,
                                                bool function_is_kernel,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(clang::Stmt const* stmt,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(xcml::compound_stmt_ptr const& scope,
                                                clang::CompoundStmt const* stmt,
                                                xcml::expr_ptr* last_value = nullptr,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(clang::CompoundStmt const* stmt,
                                                xcml::expr_ptr* last_value = nullptr,
                                                context const& = context());

    xcml::compound_stmt_ptr visit_compound_stmt(clang::CompoundStmt const* stmt,
                                                clang::Expr const* kernel,
                                                clang::FunctionDecl const* function,
                                                context const& = context());

    xcml::expr_ptr visit_expr_direct(clang::Expr const* expr, context const& = context()) {
        return visit_expr(scope_, expr, true);
    }

    xcml::expr_ptr visit_expr(clang::Expr const* expr, context const& = context()) {
        return visit_expr(scope_, expr, false);
    }

    xcml::expr_ptr visit_expr(xcml::compound_stmt_ptr const& scope, clang::Expr const* expr,
                              context const& = context()) {
        return visit_expr(scope, expr, false);
    }

    xcml::expr_ptr visit_expr(xcml::compound_stmt_ptr const& scope, clang::Expr const* expr,
                              bool direct, context const& = context());

    xcml::expr_ptr visit_expr_val(clang::Expr const* expr, context const& = context()) {
        std::cout << "---- visit_expr_val ----" << std::endl;
        std::cout << "Expression type: " << expr->getType().getAsString() << std::endl;
        std::cout << "Expression kind: " << expr->getStmtClassName() << std::endl;
        return visit_expr_val(scope_, expr);
    }

    xcml::expr_ptr visit_expr_val(xcml::compound_stmt_ptr const& scope, clang::Expr const* expr,
                                  context const& = context()) {
        auto const node = visit_expr(scope, expr);
        auto const type = expr_type(expr);

        if (!type.isNull() && type->isReferenceType()) {
            std::cout << "deref called" << std::endl;
            return deref(node);
        }
        return node;
    }

    xcml::expr_ptr visit_expr_val_with_deref_check(clang::Expr const* expr, std::string op_type) {
        auto result = visit_expr_val(expr);
        if (op_type=="plus_expr"){
            if (is_zero_dim_local_accessor_conversion(expr)) {
                std::cout << "Dereferencing 0D local accessor in binary operation" << std::endl;
                result = u::make_deref(result);
            }
        }
        return result;
    }

    bool is_zero_dim_local_accessor_conversion(clang::Expr const* expr) {
        if (auto cast_expr = clang::dyn_cast<clang::ImplicitCastExpr>(expr)) {
            if (cast_expr->getCastKind() == clang::CK_LValueToRValue) {
                if (auto sub_cast = clang::dyn_cast<clang::ImplicitCastExpr>(cast_expr->getSubExpr())) {
                    if (sub_cast->getCastKind() == clang::CK_UserDefinedConversion) {
                        // NOW CHECK: Is this actually a 0D local_accessor?
                        if (auto member_call = clang::dyn_cast<clang::CXXMemberCallExpr>(sub_cast->getSubExpr())) {
                            if (auto method = clang::dyn_cast<clang::CXXConversionDecl>(member_call->getMethodDecl())) {
                                // Check if the object is a 0D local_accessor
                                auto obj_expr = member_call->getImplicitObjectArgument();
                                auto obj_type = obj_expr->getType();
                                
                                if (auto record = obj_type->getAsCXXRecordDecl()) {
                                    if (auto template_decl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(record)) {
                                        if (template_decl->getSpecializedTemplate()->getName() == "local_accessor") {
                                            // Check dimension parameter
                                            std::cout << "check: " << std::endl;
                                            auto const& args = template_decl->getTemplateArgs();
                                            if (args.size() > 1 && args[1].getKind() == clang::TemplateArgument::Integral) {
                                                std::cout << "halo" << std::endl;
                                                return args[1].getAsIntegral().getExtValue() == 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    xcml::expr_ptr visit_expr_ref(clang::Expr const* expr, context const& = context()) {
        return visit_expr_ref(scope_, expr);
    }

    xcml::expr_ptr visit_expr_ref(xcml::compound_stmt_ptr const& scope, clang::Expr const* expr,
                                  context const& = context());

    xcml::function_call_ptr define_function(clang::Expr const* expr,
                                            clang::FunctionDecl const* decl);

protected:
    /*
    void push_expr(clang::Expr const* expr, xcml::expr_ptr const& node) {
        push_expr(scope_, expr, node);
    }
    */
    void push_expr(clang::Expr const* expr, xcml::expr_ptr const& node) {
    std::cout << "\n=== PUSH_EXPR DEBUG ===" << std::endl;
    
    // 1. Print expression info if available
    if (expr) {
        std::cout << "Clang Expression Info:" << std::endl;
        std::cout << "  Type: " << expr->getStmtClassName() << std::endl;
        std::cout << "  QualType: " << expr->getType().getAsString() << std::endl;
        
        // Check if it's a specific type we care about
        if (auto call_expr = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
            if (auto method = clang::dyn_cast<clang::CXXMethodDecl>(call_expr->getCalleeDecl())) {
                std::cout << "  Method: " << method->getNameAsString() << std::endl;
                if (auto conv = clang::dyn_cast<clang::CXXConversionDecl>(method)) {
                    std::cout << "  -> CONVERSION OPERATOR to: " 
                              << conv->getReturnType().getAsString() << std::endl;
                    std::cout << "  -> Returns reference: " 
                              << (conv->getReturnType()->isReferenceType() ? "YES" : "NO") << std::endl;
                }
            }
        } else if (auto member_expr = clang::dyn_cast<clang::MemberExpr>(expr)) {
            std::cout << "  Member: " << member_expr->getMemberNameInfo().getAsString() << std::endl;
        } else if (auto decl_ref = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
            std::cout << "  Variable: " << decl_ref->getNameInfo().getAsString() << std::endl;
        }
        
        // Print the AST structure for this expression
        std::cout << "  AST Dump:" << std::endl;
        expr->dump();
    } else {
        std::cout << "Clang Expression: NULL" << std::endl;
    }
    
    // 2. Print XCML node info
    std::cout << "\nXCML Node Info:" << std::endl;
    if (node) {
        std::cout << "  Node type: " << node->node_name() << std::endl;
        
        // Try to identify what kind of node it is
        if (auto var_ref = xcml::var_ref::dyncast(node)) {
            std::cout << "  -> VAR_REF: " << var_ref->name << std::endl;
        } else if (auto func_call = xcml::function_call::dyncast(node)) {
            if (auto func_addr = xcml::func_addr::dyncast(func_call->function)) {
                std::cout << "  -> FUNCTION_CALL: " << func_addr->name << std::endl;
            }
        } else if (auto member_ref = xcml::member_ref::dyncast(node)) {
            std::cout << "  -> MEMBER_REF: " << member_ref->member << std::endl;
        } else if (auto assign_expr = xcml::assign_expr::dyncast(node)) {
            std::cout << "  -> ASSIGNMENT" << std::endl;
        } else if (auto plus_expr = xcml::plus_expr::dyncast(node)) {
            std::cout << "  -> PLUS_EXPR" << std::endl;
        } else if (auto pointer_ref = xcml::pointer_ref::dyncast(node)) {
            std::cout << "  -> POINTER_REF (dereference)" << std::endl;
        }
        
        // Dump the XCML node structure
        std::cout << "  XCML Dump:" << std::endl;
        xcml::dump_node(node);
    } else {
        std::cout << "XCML Node: NULL" << std::endl;
    }
    
    // 3. Show context info
    std::cout << "\nContext Info:" << std::endl;
    std::cout << "  Current function: " << (current_func_ ? "available" : "none") << std::endl;
    std::cout << "  Is kernel: " << (current_is_kernel_ ? "YES" : "NO") << std::endl;
    std::cout << "  Scope symbols count: " << scope_->symbols.size() << std::endl;
    
    std::cout << "========================\n" << std::endl;
    
    // Call the original function
    push_expr(scope_, expr, node);
}
    template <class Node, class HasLoc>
    void set_loc(Node const& node, HasLoc const* expr) {
        auto& sm = ast_.getSourceManager();
        auto const loc = sm.getPresumedLoc(expr->getBeginLoc());

        if (loc.isValid()) {
            node->file = loc.getFilename();
            node->line = loc.getLine();
        } else {
            node->file = "<invalid>";
        }
    }

    template <class HasLoc>
    void push_expr(xcml::compound_stmt_ptr const& scope, HasLoc const* expr,
                   xcml::expr_ptr const& node) {
        if (expr) {
            set_loc(node, expr);
        }
        u::push_expr(scope, node);
    }

    bool is_literal(xcml::expr_ptr const& expr) {
        return xcml::int_constant::is_a(expr) || xcml::float_constant::is_a(expr);
    }

    bool is_base_cast(clang::CastExpr const* expr) {
        return expr->getCastKind() == clang::CastKind::CK_UncheckedDerivedToBase ||
               expr->getCastKind() == clang::CastKind::CK_DerivedToBase;
    }

    bool is_noop_cast(clang::CastExpr const* expr) {
        return expr->getCastKind() == clang::CK_NoOp ||
               expr->getCastKind() == clang::CK_ArrayToPointerDecay;
    }

    bool is_integral_cast(clang::CastExpr const* expr) {
        return expr->getCastKind() == clang::CK_IntegralCast ||
               expr->getCastKind() == clang::CK_IntegralToFloating ||
               expr->getCastKind() == clang::CK_FloatingCast ||
               expr->getCastKind() == clang::CK_IntegralToBoolean ||
               expr->getCastKind() == clang::CK_FloatingToIntegral;
    }

    bool is_ptr_to_ptr_cast(clang::CastExpr const* expr) {
        return expr->getCastKind() == clang::CK_NullToPointer ||
               (expr->getCastKind() == clang::CK_BitCast && expr->getType()->isPointerType() &&
                expr->getSubExpr()->getType()->isPointerType());
    }

    clang::QualType expr_type(clang::Expr const* expr);

    xcml::pointer_ref_ptr deref(xcml::expr_ptr x) {
        auto deref = xcml::new_pointer_ref();
        deref->expr = x;
        return deref;
    }

    xcml::var_ref_ptr this_ref() {
        return u::make_var_ref(info_.nm().this_name());
    }

    bool is_kernel_functor() const {
        return current_kernel_ && !clang::isa<clang::LambdaExpr>(current_kernel_);
    }

    bool is_kernel_body_processing() const {
        return current_func_ && current_is_kernel_;
    }

    bool is_this_expr(clang::Expr const* expr) {
        return expr->isImplicitCXXThis() || clang::isa<clang::CXXThisExpr>(expr);
    }

    bool is_param(clang::Expr const* expr) {
        if (auto const* dr = clang::dyn_cast<clang::DeclRefExpr>(expr->IgnoreParenImpCasts())) {
            return clang::isa<clang::ParmVarDecl>(dr->getDecl());
        }
        return false;
    }

    // bool is_captured_by_kernel(clang::Expr const* expr) {
    //     if (auto const* me = clang::dyn_cast<clang::MemberExpr>(expr)) {
    //         if (is_this_expr(me->getBase()) && is_kernel_functor() &&
    //             is_kernel_body_processing()) {
    //             return true;
    //         }

    //         return false;
    //     }

    //     if (auto const* dr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
    //         if (current_kernel_) {
    //             for (auto [vd, _] : captured_vars(info_, current_kernel_)) {
    //                 if (vd == dr->getDecl()) {
    //                     return true;
    //                 }
    //             }
    //             return false;
    //         }

    //         return false;
    //     }

    //     return false;
    // }

    clang::Expr const* get_arg(clang::Expr const* expr, unsigned i) {
        PUSH_CONTEXT(expr);

        if (auto const* call = clang::dyn_cast<clang::CallExpr>(expr)) {
            return call->getArg(i);
        }
        if (auto const* ctor = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
            return ctor->getArg(i);
        }

        not_supported(expr, ast_);
    }

    size_t num_args(clang::Expr const* expr) {
        PUSH_CONTEXT(expr);

        if (auto const* call = clang::dyn_cast<clang::CallExpr>(expr)) {
            return call->getNumArgs();
        }
        if (auto const* ctor = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
            return ctor->getNumArgs();
        }

        not_supported(expr, ast_);
    }

    bool is_accessor(clang::QualType type) const {
        accessor_type dummy;
        return ::is_accessor(type, dummy);
    }

    xcml::function_call_ptr make_call_expr(clang::Expr const* expr,
                                           xcml::expr_ptr this_ref = nullptr,
                                           bool this_is_ptr = false);

    void construct_inherit(xcml::compound_stmt_ptr const& scope, clang::Decl const* for_loc,
                           xcml::expr_ptr const& this_ref, bool this_is_ptr,
                           clang::CXXConstructorDecl const* ctor,
                           clang::FunctionDecl const* decl) {
        auto record = ctor->getParent();
        info_.check_device_copyable(record);

        if (ctor->isDefaultConstructor() && ctor->isTrivial()) {
            return;
        }

        auto call = define_function(nullptr, ctor);
        xcml::expr_ptr this_expr = this_is_ptr ? this_ref : u::make_addr_of(this_ref);

        call->arguments.push_back(this_expr);
        for (auto param : decl->parameters()) {
            call->arguments.push_back(u::make_var_ref(info_.rename_sym(param)));
        }

        push_expr(scope, for_loc, call);
    }

    void construct(xcml::compound_stmt_ptr scope, xcml::expr_ptr const& this_ref,
                   bool this_is_ptr, clang::CXXConstructExpr const* expr,
                   xcml::var_ref_ptr const& ref) {
        auto ctor = expr->getConstructor();
        auto record = ctor->getParent();

        info_.check_device_copyable(record);

        if (ctor->isDefaultConstructor() && ctor->isTrivial()) {
            return;
        }

        if (ctor->isCopyOrMoveConstructor() && record->isTriviallyCopyable()) {
            auto asg = u::assign_expr(ref, visit_expr_val(expr->getArg(0)));
            push_expr(scope, expr, asg);
        } else {
            auto call = make_call_expr(expr, this_ref, this_is_ptr);
            push_expr(scope, expr, call);
        }
    }

    bool is_runtime_extern(clang::FunctionDecl const* decl) {
        return has_annotate(decl, "charm_sycl_extern_runtime");
    }

    void add_to_scope(xcml::compound_stmt_ptr scope, clang::DeclStmt const* stmt) {
        PUSH_CONTEXT(stmt);

        for (auto decl : stmt->decls()) {
            PUSH_CONTEXT(decl);

            visit_decl(scope, decl);
        }
    }

    xcml::expr_ptr visit_apsint(llvm::APSInt const& val) {
        if (val.isSignedIntN(64)) {
            return u::lit(val.getSExtValue());
        } else if (val.isIntN(64)) {
            return u::lit(val.getExtValue());
        }
        return nullptr;
    }

    transform_info& info_;
    clang::ASTContext& ast_;
    xcml::compound_stmt_ptr scope_;
    clang::Expr const* current_kernel_;
    clang::FunctionDecl const* current_func_;
    bool current_is_kernel_;
};

template <class Derived, class RetTy>
struct decl_visitor_base : clang::ConstDeclVisitor<Derived, RetTy>, visitor_base {
    using visitor_base::visitor_base;

    void Visit(clang::Decl const* decl, context const& = context()) {
        return clang::ConstDeclVisitor<Derived, RetTy>::Visit(decl);
    }

    void VisitDecl(clang::Decl const* decl, context const& = context()) {
        not_supported(decl, info_.ctx());
    }
};

template <class Derived, class RetTy, class Arg = void>
struct stmt_visitor_base : clang::ConstStmtVisitor<Derived, RetTy, Arg, context const&>,
                           visitor_base {
    using visitor_base::visitor_base;

    RetTy Visit(clang::Stmt const* stmt, Arg const& arg = Arg(),
                context const& ctx = context()) {
        return clang::ConstStmtVisitor<Derived, RetTy, Arg, context const&>::Visit(stmt, arg,
                                                                                   ctx);
    }

    RetTy VisitStmt(clang::Stmt const* stmt, Arg const& = Arg(), context const& = context()) {
        not_supported(stmt, info_.ctx());
    }

    RetTy VisitExpr(clang::Expr const* expr, Arg const& = Arg(), context const& = context()) {
        not_supported(expr, info_.ctx());
    }
};

template <class Derived, class RetTy>
struct stmt_visitor_base<Derived, RetTy, void>
    : clang::ConstStmtVisitor<Derived, RetTy, context const&>, visitor_base {
    using visitor_base::visitor_base;

    RetTy Visit(clang::Stmt const* stmt, context const& ctx = context()) {
        return clang::ConstStmtVisitor<Derived, RetTy, context const&>::Visit(stmt, ctx);
    }

    RetTy VisitStmt(clang::Stmt const* stmt, context const& = context()) {
        not_supported(stmt, info_.ctx());
    }

    RetTy VisitExpr(clang::Expr const* expr, context const& = context()) {
        not_supported(expr, info_.ctx());
    }
};
