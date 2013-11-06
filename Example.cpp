#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info

public:
    explicit ExampleVisitor(CompilerInstance *CI) 
      : astContext(&(CI->getASTContext())) // initialize private members
    {
    }

    virtual bool VisitObjCMessageExpr(ObjCMessageExpr *E) {
      if (E->getReceiverKind() == ObjCMessageExpr::Class) {
        QualType ReceiverType = E->getClassReceiver();
        Selector Sel = E->getSelector();
        string TypeName = ReceiverType.getAsString();
        string SelName = Sel.getAsString();
        if (TypeName == "Observer" && SelName == "observerWithTarget:action:") {
          Expr *Receiver = E->getArg(0)->IgnoreParenCasts();
          ObjCSelectorExpr* SelExpr = cast<ObjCSelectorExpr>(E->getArg(1)->IgnoreParenCasts());
          Selector Sel = SelExpr->getSelector();
          if (const ObjCObjectPointerType *OT = Receiver->getType()->getAs<ObjCObjectPointerType>()) {
            ObjCInterfaceDecl *decl = OT->getInterfaceDecl();
            if (! decl->lookupInstanceMethod(Sel)) {
              errs() << "Warning: class " << TypeName << " does not implement selector " << Sel.getAsString() << "\n";
              SourceLocation Loc = E->getExprLoc();
              PresumedLoc PLoc = astContext->getSourceManager().getPresumedLoc(Loc);
              errs() << "in " << PLoc.getFilename() << " <" << PLoc.getLine() << ":" << PLoc.getColumn() << ">\n";
            }
          }
        }
      }
      return true;
    }
};



class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor *visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

};



class ExampleFrontendAction : public ASTFrontendAction {
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return new ExampleASTConsumer(&CI); // pass CI pointer to ASTConsumer
    }
};



int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>());
    return result;
}
