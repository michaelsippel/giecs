
#pragma once

namespace giecs
{

struct EvaluatorBase;

struct ProgramBase
{
    virtual ~ProgramBase() {}
    virtual EvaluatorBase* createEvaluator(void)
    {
        return nullptr;
    }

    virtual ProgramBase* next(void)
    {
        return nullptr;
    }
}; // struct ProgramBase

struct EvaluatorBase
{
    virtual ~EvaluatorBase() {}
    virtual void eval(ProgramBase&) {}
}; // struct EvaluatorBase

template <typename Core, typename Program>
struct Evaluator : public EvaluatorBase
{
    void eval(ProgramBase& prg)
    {
        Program& tprg = * static_cast<Program*>(&prg);
        Core::eval(tprg.program(), tprg.data());

        //   delete this;
    }
}; // struct Evaluator

template <typename Core, typename Derived>
struct Program : public ProgramBase
{
    EvaluatorBase* createEvaluator(void) final
    {
        return new Evaluator<Core, Derived>();
    }
}; // struct Program

void eval(ProgramBase* prg)
{
    EvaluatorBase* e;
    while(prg && (e = prg->createEvaluator()))
    {
        e->eval(*prg);
        prg = prg->next();
    }
}

} // namespace giecs

