/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

/*
 * There is no standard implementation of a hash for tuples, and it's a pain to
 * make one, so that it works on compilers that do not support variadic
 * templates. Therefore, we fallback to std::map when keys are tuples.
 */
#include <functional>
#include <map> 
#include <tuple>

#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <nc/common/Types.h>

#include "CalleeId.h"

namespace nc {
namespace core {
namespace ir {

class Call;
class Function;
class Statement;
class Return;

namespace dflow {
    class Dataflow;
}

namespace calling {

class CallHook;
class CallSignature;
class Convention;
class Conventions;
class EntryHook;
class FunctionSignature;
class ReturnHook;
class Signature;
class Signatures;

/**
 * This class is responsible for instrumenting functions with special hooks
 * that take care of handling calling-convention-specific stuff.
 */
class Hooks {
    /** Assigned calling conventions. */
    const Conventions &conventions_;

    /** Signatures of functions. */
    const Signatures &signatures_;

public:
    /** Type for the calling convention detector callback. */
    typedef std::function<void(const CalleeId &)> ConventionDetector;

private:
    /** Calling convention detector. */
    ConventionDetector conventionDetector_;

    /** Hooks inserted into a given function. */
    boost::unordered_map<Function *, std::vector<Statement *>> insertedHooks_;

    /** All entry hooks ever created. */
    std::map<std::tuple<const Function *, const Convention *, const FunctionSignature *>, std::unique_ptr<EntryHook>> entryHooks_;

    /** Mapping from a function to the last entry hook used for instrumenting it. */
    boost::unordered_map<const Function *, EntryHook *> lastEntryHooks_;

    /** All call hooks ever created. */
    std::map<std::tuple<const Call *, const Convention *, const CallSignature *, boost::optional<ByteSize>>, std::unique_ptr<CallHook>> callHooks_;

    /** Mapping from a call to the last call hook used for instrumenting it. */
    boost::unordered_map<const Call *, CallHook *> lastCallHooks_;

    /** All return hooks ever created. */
    std::map<std::tuple<const Return *, const Convention *, const FunctionSignature *>, std::unique_ptr<ReturnHook>> returnHooks_;

    /** Mapping from a return to the last return hook used for instrumenting it. */
    boost::unordered_map<const Return *, ReturnHook *> lastReturnHooks_;

public:
    /**
     * Constructor.
     *
     * \param conventions Assigned calling conventions.
     * \param signatures Known signatures of functions.
     */
    Hooks(const Conventions &conventions, const Signatures &signatures);

    /**
     * Destructor.
     */
    ~Hooks();

    /**
     * \return Assigned calling conventions.
     */
    const Conventions &conventions() const { return conventions_; }

    /**
     * Sets the function being called when a calling convention for a particular
     * callee is requested, but currently unknown. It is assumed that this function
     * will detect this convention and modify Conventions object passed to the
     * constructor of this Hooks object.
     *
     * \param detector Calling convention detector.
     */
    void setConventionDetector(ConventionDetector detector) {
        conventionDetector_ = std::move(detector);
    }

    /**
     * \param calleeId Callee id.
     *
     * \return Pointer to the calling convention used for calls to given address. Can be NULL.
     */
    const Convention *getConvention(const CalleeId &calleeId) const;

    /**
     * \param function Valid pointer to a function.
     *
     * \return Pointer to the last EntryHook used for instrumenting this function.
     *         Can be NULL.
     */
    const EntryHook *getEntryHook(const Function *function) const;

    /**
     * \param call Valid pointer to a call statement.
     *
     * \return Pointer to the last CallHook used for instrumenting this call.
     *         Can be NULL.
     */
    const CallHook *getCallHook(const Call *call) const;

    /**
     * \param function Valid pointer to a function.
     * \param ret Valid pointer to a return statement.
     *
     * \return Pointer to the last ReturnHook used for instrumenting this return.
     *         Can be NULL.
     */
    const ReturnHook *getReturnHook(const Return *ret) const;

    /**
     * Inserts callback statements into the function. When executed by
     * the dataflow analyzer, these callback statements will insert
     * calling-convention-specific code in the function's entry, and
     * at call and return sites.
     *
     * \param function Valid pointer to a function.
     * \param dataflow Dataflow information storage used for the analysis.
     */
    void instrument(Function *function, const dflow::Dataflow *dataflow);

    /**
     * Undoes instrumentation of a function.
     *
     * \param function Valid pointer to a function.
     */
    void deinstrument(Function *function);

    /**
     * Undoes instrumentation of all the functions.
     */
    void deinstrumentAll();

private:
    /**
     * Creates an EntryHook (if not done yet) and instruments the function with it.
     * If the function was previously instrumented, deinstruments it.
     *
     * \param function Valid pointer to a function.
     */
    void instrumentEntry(Function *function);

    /**
     * Undoes the instrumentation of a function's entry, if performed before.
     * Otherwise, does nothing.
     *
     * \param function Valid pointer to a function.
     */
    void deinstrumentEntry(Function *function);

    /**
     * Creates a CallHook (if not done yet) and instruments the call with it.
     * If the call was previously instrumented, deinstruments it.
     *
     * \param call Valid pointer to a call.
     * \param calledAddress The address being called or boost::none.
     */
    void instrumentCall(Call *call, const boost::optional<ByteAddr> &calledAddress);

    /**
     * Undoes the instrumentation of a call, if performed before.
     * Otherwise, does nothing.
     *
     * \param function Valid pointer to a call.
     */
    void deinstrumentCall(Call *call);

    /**
     * Creates a ReturnHook (if not done yet) and instruments the return with it.
     * If the return was previously instrumented, deinstruments it.
     *
     * \param return Valid pointer to a return statement.
     */
    void instrumentReturn(Return *ret);

    /**
     * Undoes the instrumentation of a return statement, if performed before.
     * Otherwise, does nothing.
     *
     * \param ret Valid pointer to a return statement.
     */
    void deinstrumentReturn(Return *ret);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
