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

#include <memory> /* For std::unique_ptr. */

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <QObject>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>
#include <nc/common/Range.h> /* For nc::find(). */
#include <nc/common/Types.h>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace nc {
namespace core {

namespace arch {
    class Instructions;
}

namespace image {
    class ByteSource;
    class Section;
}

namespace ir {
    class Function;
    class Functions;
    class Program;

    namespace calling {
        class Conventions;
        class Hooks;
        class Signatures;
    }
    namespace cflow {
        class Graph;
    }
    namespace dflow {
        class Dataflows;
    }
    namespace misc {
        class TermToFunction;
    }
    namespace types {
        class Types;
    }
    namespace liveness {
        class Liveness;
    }
    namespace vars {
        class Variables;
    }
}

namespace likec {
    class Tree;
}

class Module;

/**
 * This class stores all the information that is required and produced during decompilation.
 */
class Context: public QObject {
    Q_OBJECT

    std::shared_ptr<Module> module_; ///< Module being decompiled.
    std::shared_ptr<const arch::Instructions> instructions_; ///< Instructions being decompiled.
    std::unique_ptr<ir::Program> program_; ///< Program.
    std::unique_ptr<ir::Functions> functions_; ///< Functions.
    std::unique_ptr<ir::calling::Conventions> conventions_; ///< Assigned calling conventions.
    std::unique_ptr<ir::calling::Hooks> hooks_; ///< Hooks of calling conventions.
    std::unique_ptr<ir::calling::Signatures> signatures_; ///< Signatures.
    std::unique_ptr<ir::dflow::Dataflows> dataflows_; ///< Dataflows.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::liveness::Liveness> > livenesses_; ///< Liveness information.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::types::Types> > types_; ///< Information about types.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::vars::Variables> > variables_; ///< Reconstructed variables.
    boost::unordered_map<const ir::Function *, std::unique_ptr<ir::cflow::Graph> > regionGraphs_; ///< Region graphs.
    std::unique_ptr<likec::Tree> tree_; ///< Representation of LikeC program.
    std::unique_ptr<ir::misc::TermToFunction> termToFunction_; ///< Term to function mapping.
    LogToken logToken_; ///< Log token.
    CancellationToken cancellationToken_; ///< Cancellation token.

public:
    /**
     * Class constructor.
     */
    Context();

    /**
     * Class destructor.
     */
    ~Context();

    /**
     * \return Valid pointer to the module being decompiled.
     */
    std::shared_ptr<Module> module() const { return module_; }

    /**
     * Sets the module.
     *
     * \param module Valid pointer to a module.
     */
    void setModule(const std::shared_ptr<Module> &module);

    /**
     * \returns Valid pointer to the instructions being decompiled.
     */
    const std::shared_ptr<const arch::Instructions> &instructions() const { return instructions_; }

    /**
     * Sets the set instructions of the executable file.
     *
     * \param instructions New set of instructions.
     */
    void setInstructions(const std::shared_ptr<const arch::Instructions> &instructions);

    /**
     * Sets the control flow graph of the program.
     *
     * \param program Valid pointer to the program CFG.
     */
    void setProgram(std::unique_ptr<ir::Program> program);

    /**
     * \return Pointer to the program. Can be NULL.
     */
    const ir::Program *program() const { return program_.get(); }

    /**
     * Sets the set of functions.
     *
     * \param functions Valid pointer to the set of functions.
     */
    void setFunctions(std::unique_ptr<ir::Functions> functions);

    /**
     * \return Pointer to the set of functions. Can be NULL.
     */
    ir::Functions *functions() const { return functions_.get(); }

    /**
     * Sets the assigned calling conventions.
     *
     * \param conventions Valid pointer to the assigned calling conventions.
     */
    void setConventions(std::unique_ptr<ir::calling::Conventions> conventions);

    /**
     * \return Valid pointer to the information on calling conventions of functions.
     */
    ir::calling::Conventions *conventions() const { return conventions_.get(); }

    /**
     * Sets the calling conventions hooks.
     *
     * \param hooks Valid pointer to the hooks information.
     */
    void setHooks(std::unique_ptr<ir::calling::Hooks> hooks);

    /**
     * \return Valid pointer to the information on calling conventions of functions.
     */
    ir::calling::Hooks *hooks() const { return hooks_.get(); }

    /**
     * Sets the reconstructed signatures.
     *
     * \param signatures Valid pointer to the signatures.
     */
    void setSignatures(std::unique_ptr<ir::calling::Signatures> signatures);

    /**
     * \return Pointer to the signatures of functions. Can be NULL, if not set.
     */
    const ir::calling::Signatures *signatures() const { return signatures_.get(); }

    /**
     * Sets the term to function mapping.
     *
     * \param termToFunction Valid pointer to the term to function mapping.
     */
    void setTermToFunction(std::unique_ptr<ir::misc::TermToFunction> termToFunction);

    /**
     * \return Valid pointer to the term to function mapping.
     */
    const ir::misc::TermToFunction *termToFunction() const { return termToFunction_.get(); }

    /**
     * Sets the dataflows.
     *
     * \param[in] dataflows Dataflows.
     */
    void setDataflows(std::unique_ptr<ir::dflow::Dataflows> dataflows);

    /**
     * \return Pointer to the dataflows.
     */
    ir::dflow::Dataflows *dataflows() const { return dataflows_.get(); }

    /**
     * Sets the dataflow information for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] liveness Valid pointer to the liveness information.
     */
    void setLiveness(const ir::Function *function, std::unique_ptr<ir::liveness::Liveness> liveness);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the liveness information. Can be NULL.
     */
    const ir::liveness::Liveness *getLiveness(const ir::Function *function) const;

    /**
     * Sets the type information for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] types Valid pointer to the type information.
     */
    void setTypes(const ir::Function *function, std::unique_ptr<ir::types::Types> types);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the type information for a given function. Can be NULL.
     */
    const ir::types::Types *getTypes(const ir::Function *function) const;

    /**
     * Sets the information about variables for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] variables Valid pointer to the information about variables.
     */
    void setVariables(const ir::Function *function, std::unique_ptr<ir::vars::Variables> variables);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Pointer to the information about variables for a given function. Can be NULL.
     */
    const ir::vars::Variables *getVariables(const ir::Function *function) const;

    /**
     * Sets the region graph for a function.
     *
     * \param[in] function Valid pointer to a function.
     * \param[in] graph Valid pointer to the region graph.
     */
    void setRegionGraph(const ir::Function *function, std::unique_ptr<ir::cflow::Graph> graph);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Valid pointer to the region graph of the given function.
     */
    const ir::cflow::Graph *getRegionGraph(const ir::Function *function) const;

    /**
     * Sets the LikeC tree.
     *
     * \param tree Valid pointer to the LikeC tree.
     */
    void setTree(std::unique_ptr<likec::Tree> tree);

    /**
     * \return The LikeC tree. Can be NULL.
     */
    likec::Tree *tree() const { return tree_.get(); }

    /**
     * Sets cancellation token.
     *
     * \param token Cancellation token.
     */
    void setCancellationToken(const CancellationToken &token) { cancellationToken_ = token; }

    /**
     * \return Cancellation token.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    /**
     * Sets the log token.
     *
     * \param token log token.
     */
    void setLogToken(const LogToken &token) { logToken_ = token; }

    /**
     * \return Log token.
     */
    const LogToken &logToken() const { return logToken_; }

    Q_SIGNALS:

    /**
     * Signal emitted when the set of instructions is changed.
     */
    void instructionsChanged();

    /**
     * Signal emitted when C tree is computed.
     */
    void treeChanged();
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
