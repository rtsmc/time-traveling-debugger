# 6.4.2 Stakeholder Needs and Requirements Definition process

## 1. Prepare for Stakeholder Needs and Requirements Definition

### a. Identify the stakeholders who have an interest in the software system throughout its lifecycle.

**IEEE Outcome:** a

**XP Implementation:** "Include on the team people with all the skills and perspectives necessary for the project to succeed. This is really nothing more than the old idea of cross-functional teams. The name reflects the purpose of the practice, a sense of wholeness on the team, the ready availability of all the resources necessary to succeed."

**Project Implementation:** Our time-traveling debugger team consists of four developers with complementary skills. We identified our primary stakeholders as: (1) ourselves as developers who need debugging tools, (2) our course instructor as the evaluator, (3) future students who may use or extend the debugger, and (4) the broader Python developer community who could benefit from time-travel debugging capabilities. Each team member brings different expertise: C extension development, Python scripting, CLI interface design, and testing/documentation.

### b. Define the stakeholder needs and requirements definition strategy.

**IEEE Outcome:** d

**XP Implementation:** "Plan using units of customer-visible functionality. 'Handle five times the traffic with the same response time.' 'Provide a two-click way for users to dial frequently used numbers.' As soon as a story is written, try to estimate the development effort necessary to implement it."

**Project Implementation:** We defined our requirements through user stories written on index cards during our initial planning session. Examples include: "As a developer, I want to set breakpoints before execution so I can pause at specific lines," "As a debugger user, I want to step backwards through execution history," and "As a student debugging loops, I want breakpoints to persist across iterations." Each story was estimated during our weekly planning meetings, with the team collectively deciding which stories to implement each iteration.

## 2. Define stakeholder needs

### a. Define context of use within the concept of operations and the preliminary life cycle concepts.

**IEEE Outcome:** b

**XP Implementation:** "Develop in an open space big enough for the whole team. Meet the need for privacy and 'owned' space by having small private spaces nearby or by limiting work hours so team members can get their privacy needs met elsewhere."

**Project Implementation:** We work together in our senior project lab space where all team members can see the current state of the debugger. Our informative workspace includes a whiteboard showing our current story cards organized into "Done," "This Week," and "Future" columns. We identified our context of use as: developers working on Python applications who encounter bugs in loops, complex control flow, or need to understand execution history. The debugger operates in a command-line environment where developers already spend significant time.

### b. Identify stakeholder needs.

**IEEE Outcome:** d

**XP Implementation:** "I took two lessons from that experience. One is that no matter what the client says the problem is, it is always a people problem. Technical fixes alone are not enough."

**Project Implementation:** Through conversations with classmates and reflection on our own debugging experiences, we identified key needs: (1) the ability to examine program state at any point in execution history, (2) minimal code modification to use the debugger (no manual instrumentation), (3) interactive breakpoint setting without rerunning the program, (4) clear visualization of variables at each step, and (5) support for debugging loops where breakpoints should trigger on every iteration. We realized the underlying need wasn't just "see execution history" but rather "understand how my code reached this broken state."

### c. Prioritize and down-select needs.

**IEEE Outcome:** d, e

**XP Implementation:** "One feature of XP-style planning is that stories are estimated very early in their life. This gets everyone thinking about how to get the greatest return from the smallest investment."

**Project Implementation:** During our first weekly cycle, we prioritized needs based on effort and value. Our highest priority was the C extension that captures execution traces (foundational capability). Second priority was interactive breakpoint setting via the CLI. Third was the post-execution trace viewer. We explicitly de-prioritized features like multi-threaded debugging and remote debugging sessions, acknowledging these as valuable but beyond our current scope. This prioritization allowed us to deliver a working debugger with core functionality early, then iterate on enhancements.

### d. Define the stakeholder needs and rationale.

**IEEE Outcome:** d

**XP Implementation:** "Software development has been steered wrong by the word 'requirement', defined in the dictionary as 'something mandatory or obligatory.' The word carries a connotation of absolutism and permanence, inhibitors to embracing change."

**Project Implementation:** Rather than rigid "requirements," we documented evolving needs with rationales. For example, "Step backwards through execution" was needed because "developers often realize a bug exists several steps after it occurs, and want to examine the exact moment the state became incorrect without rerunning the entire program." We explicitly acknowledged that our understanding of stakeholder needs would evolve as we used the debugger ourselves, practicing what XP calls "improvement" - getting started and refining based on experience.

## 3. Develop the operational concept and other life cycle concepts

### a. Define a representative set of scenarios to identify the required capabilities that correspond to anticipated operational and other life cycle concepts.

**IEEE Outcome:** b

**XP Implementation:** "One feature of XP-style planning is that stories are estimated very early in their life. This gets everyone thinking about how to get the greatest return from the smallest investment."

**Project Implementation:** We developed concrete scenarios documented in our test files. For example, `test_clean_loops.py` represents the scenario: "A developer debugging a loop that produces wrong output needs to examine variable values at each iteration." The scenario walks through setting a breakpoint inside the loop, running the program, and observing the breakpoint trigger multiple times. Another scenario in `test_import_main.py` addresses: "A developer whose bug involves interaction between two modules needs to set breakpoints in both the main file and imported helper file." These scenarios directly informed our design decisions about breakpoint matching and file path resolution.

### b. Identify the factors affecting interactions between users and the system.

**IEEE Outcome:** b

**XP Implementation:** "Make your workspace about your work. An interested observer should be able to walk into the team space and get a general idea of how the project is going in fifteen seconds."

**Project Implementation:** We identified several interaction factors: (1) developers typically work in specific directory structures, so our debugger needed to handle both absolute and relative file paths for breakpoints; (2) developers use different terminals with varying color support, so we added ANSI color codes that degrade gracefully; (3) developers expect debugger commands to be brief since they're typed repeatedly, so we provided single-letter shortcuts (`n`, `b`, `c`) alongside full commands; (4) developers need to maintain context while debugging, so our trace viewer displays the current execution number and position (e.g., "[25/150]") at each prompt.

## 4. Transform stakeholder needs into stakeholder requirements

### a. Identify the constraints on a system solution.

**IEEE Outcome:** c

**XP Implementation:** "Work only as many hours as you can be productive and only as many hours as you can sustain. Burning yourself out unproductively today and spoiling the next two days' work isn't good for you or the team."

**Project Implementation:** We identified several constraints: (1) The debugger must be implemented as a Python C extension to access CPython's tracing API, limiting us to developers with both C and Python knowledge; (2) Trace files are limited by available disk space, constraining how many execution steps we can record (we set MAX_LINES to 100,000); (3) The debugger requires Python 3.9+ to access frame inspection APIs; (4) We're constrained by our 10-week academic quarter, requiring us to prioritize features that provide immediate value; (5) As students, we work approximately 10-15 hours per week per person on the project, fitting work into sustainable schedules alongside other courses.

### b. Identify the stakeholder requirements and functions that relate to critical quality characteristics.

**IEEE Outcome:** e, f

**XP Implementation:** "Write a failing automated test before changing any code. Test-first programming addresses many problems at once: Scope creep, Coupling and cohesion, Trust, Rhythm."

**Project Implementation:** Critical quality characteristics for our debugger include: (1) **Correctness** - the trace must accurately capture execution order and variable values. We test this through `test_variables.py` which verifies variables are captured at each step. (2) **Performance** - trace recording shouldn't slow execution prohibitively. We target the "ten-minute build" XP principle, ensuring our test suite completes quickly even with tracing enabled. (3) **Usability** - commands must be intuitive. We validate this through our own usage and by having classmates try the debugger. (4) **Reliability** - the debugger must handle crashes gracefully, captured in `test_crash.py` which ensures trace files are written even when the debugged program fails.

### c. Obtain explicit agreement with designated stakeholders on the stakeholder requirements.

**IEEE Outcome:** g

**XP Implementation:** "Plan work a week at a time. Have a meeting at the beginning of every week. During this meeting: Review progress to date, including how actual progress for the previous week matched expected progress. Have the customers pick a week's worth of stories to implement this week."

**Project Implementation:** During our weekly planning meetings, we review the previous week's completed stories and select new stories for the coming week. Our "customer" is primarily ourselves and our instructor, so we achieve agreement through consensus. For example, in Week 3 we agreed to prioritize the interactive CLI (`idebug.py`) over the post-execution viewer because the team felt interactive debugging provided more immediate value. This agreement was documented by moving story cards from "Future" to "This Week" on our physical story wall, making the commitment visible and explicit to all team members.

### d. Maintain traceability of stakeholder needs and requirements.

**IEEE Outcome:** i

**XP Implementation:** "Give stories short names in addition to a short prose or graphical description. Write the stories on index cards and put them on a frequently-passed wall."

**Project Implementation:** We maintain traceability through our story cards and test files. Each story card has a short name ("Breakpoint Persistence," "Step Back," "Cross-File Breakpoints") and references specific test files that validate the feature. For example, "Breakpoint Persistence" traces to `test_clean_loops.py` which demonstrates breakpoints triggering on multiple loop iterations. Our README.md documents which features address which initial needs. When we encounter questions like "Why did we implement basename matching for breakpoints?", we can trace back to the stakeholder need for "minimal friction when setting breakpoints" which was identified during Week 2 planning.

### e. Provide key artifacts and information items that have been selected for baselines.

**IEEE Outcome:** h

**XP Implementation:** "In any plan, include some minor tasks that can be dropped if you get behind. You can always add more stories later and deliver more than you promised."

**Project Implementation:** Our baseline artifacts include: (1) The core C extension (`debugger.c`) implementing the tracing API; (2) Test files (`test_clean_simple.py`, `test_clean_loops.py`) representing our acceptance criteria; (3) The README.md documenting installation and usage; (4) The setup.py enabling distribution. We maintain these in our git repository with clear commit messages. We deliberately included "slack" in our plan - features like the trace file viewer and cross-file debugging were marked as "if time permits." This allowed us to deliver the essential interactive debugger early while reserving enhancements for later iterations if time allowed.

## 5. Analyze stakeholder requirements

### a. Analyze the complete set of stakeholder requirements.

**IEEE Outcome:** e

**XP Implementation:** "Planning is a form of necessary waste. It doesn't create much value all by itself. Work on gradually reducing the percentage of time you spend planning."

**Project Implementation:** We analyze requirements through weekly retrospectives and continuous use of our own debugger. During Week 4, we discovered our initial requirements didn't account for the complexity of Python's frame object changes between Python versions. This analysis led us to add compatibility code in `debugger.c` with preprocessor directives for Python 3.9-3.10 vs 3.11+. Rather than extensive upfront analysis, we analyze requirements as we encounter implementation challenges, keeping planning lean but sufficient. Our weekly planning sessions typically take 1-2 hours, which we've gradually reduced from the 3-4 hours we initially spent.

### b. Define critical performance measures that enable the assessment of technical achievement.

**IEEE Outcome:** f

**XP Implementation:** "Automatically build the whole system and run all of the tests in ten minutes. A build that takes longer than ten minutes will be used much less often, missing the opportunity for feedback."

**Project Implementation:** Our critical performance measures include: (1) **Build time** - `python3 setup.py build_ext --inplace` completes in under 10 seconds; (2) **Test execution time** - our full test suite (running programs with tracing enabled) completes in under 2 minutes; (3) **Trace file size** - we measure trace file growth rate and found that typical debugging sessions generate ~50KB per 100 execution steps; (4) **Breakpoint response time** - the debugger should pause at a breakpoint within 100ms of the line being hit; (5) **Memory usage** - our in-memory trace history is capped at 1000 entries to prevent excessive memory consumption during long debugging sessions.

### c. Feed back the analyzed requirements to applicable stakeholders to validate that their needs and expectations have been adequately captured and expressed.

**IEEE Outcome:** g

**XP Implementation:** "I took two lessons from that experience. One is that no matter what the client says the problem is, it is always a people problem. Technical fixes alone are not enough. The other lesson I took was how important it is to sit together, to communicate with all our senses."

**Project Implementation:** We validate requirements through pair programming and "dogfooding" (using our own debugger). When implementing the step-back feature, we had a teammate who wasn't involved in that story try using it to debug one of their own programs. Their immediate feedback - "I don't understand what execution number I'm at" - led us to add the execution counter display. We also demonstrated our debugger to classmates in other capstone teams, gathering feedback like "I keep forgetting the command to step back" which led us to add the single-letter 'b' shortcut. This continuous feedback cycle validates that we're building what stakeholders actually need.

### d. Resolve stakeholder requirements issues.

**IEEE Outcome:** c, d

**XP Implementation:** "Pairing doesn't mean that you can't think alone. People need both companionship and privacy. If you need to work on an idea alone, go do it. Then come back and check in with your team."

**Project Implementation:** We encountered conflicting requirements around breakpoint syntax. Some team members wanted to specify breakpoints with just line numbers (like GDB), while others wanted file:line syntax (like VS Code). We resolved this by implementing a hybrid approach: in the interactive CLI (`idebug.py`), breakpoints require both file and line since you set them before execution starts; in the post-execution viewer (`traceviewer`), you can jump to a line number since the file context is already known. This resolution emerged from a pair programming session where we prototyped both approaches and discussed the trade-offs, demonstrating XP's balance between collaboration and individual exploration.

## 6. Manage the stakeholder needs and requirements definition

### a. Obtain explicit agreement with designated stakeholders on the stakeholder requirements.

**IEEE Outcome:** g

**XP Implementation:** "Plan work a quarter at a time. Once a quarter reflect on the team, the project, its progress, and its alignment with larger goals."

**Project Implementation:** At the quarter mark (Week 5), we held a longer reflection meeting with our instructor present. We reviewed our original story cards against what we'd implemented and gained explicit agreement on our adjusted scope. We agreed to defer the planned "conditional breakpoints" feature in favor of polishing the core time-travel capabilities. Our instructor confirmed that a debugger demonstrating solid interactive breakpointing and step-back functionality would meet capstone requirements, even without every initially-envisioned feature. This quarterly review aligned our trajectory with the larger goal of demonstrating engineering capability, not just feature completeness.

### b. Maintain traceability of the system/software requirements.

**IEEE Outcome:** i

**XP Implementation:** "Give stories short names in addition to a short prose or graphical description. Write the stories on index cards and put them on a frequently-passed wall."

**Project Implementation:** Our story wall provides visual traceability. Each card in the "Done" column links to commits in our git repository via commit messages that reference story names (e.g., "Implement Breakpoint Persistence - closes #story-3"). Our test files provide executable traceability - `test_clean_loops.py` directly traces to the "Loop Debugging" story. When someone asks "Do we support nested loops?", we can point to the specific test and the story card that motivated it. The physical wall makes this traceability visible at a glance, whereas a digital tool would require navigation.

### c. Provide key artifacts and information items that have been selected for baselines.

**IEEE Outcome:** h

**XP Implementation:** "In any plan, include some minor tasks that can be dropped if you get behind. You can always add more stories later and deliver more than you promised."

**Project Implementation:** We established three baseline milestones: (1) **Week 3 baseline**: Basic trace recording working, validated by running `test.py` and generating a trace file; (2) **Week 6 baseline**: Interactive breakpoints working, validated by `test_clean_simple.py` with interactive breakpoint setting; (3) **Week 9 baseline**: Complete time-travel debugging, validated by all test files passing. Each baseline includes our test files, README updates, and working executables committed to git with tags. The "slack" features we included (like the informative workspace suggestions in README) could be dropped without breaking these baselines, giving us flexibility while ensuring core functionality remained stable.

# 6.4.4 Architecture Definition process

## 1. Prepare for architecture definition

### a. Review pertinent information and identify key drivers of the architecture.

**IEEE Outcome:** a

**XP Implementation:** "Invest in the design of the system every day. Strive to make the design of the system an excellent fit for the needs of the system that day."

**Project Implementation:** Our key architectural drivers emerged from daily design discussions: (1) **Performance** - tracing must impose minimal overhead, driving the decision to implement core tracing in C rather than pure Python; (2) **Compatibility** - supporting Python 3.9-3.12+ required architecture decisions about frame object access patterns; (3) **Usability** - developers shouldn't modify their code to use the debugger, leading to our "wrapper script" architecture where `idebug.py` invokes the user's program; (4) **Extensibility** - supporting future features like remote debugging influenced our decision to separate trace capture (C extension) from trace viewing (separate executables). We review these drivers during each weekly cycle, ensuring our design remains aligned with current needs.

### b. Identify stakeholder concerns.

**IEEE Outcome:** a

**XP Implementation:** "Include on the team people with all the skills and perspectives necessary for the project to succeed."

**Project Implementation:** Our team includes members with different concerns that shape architecture: (1) Our C-experienced member emphasized the concern for **memory safety**, leading to careful buffer management in `debugger.c`; (2) Our CLI-focused member raised concerns about **user experience**, driving the design of colored output and command shortcuts; (3) Our testing-focused member's concern for **test coverage** led to separating test files by feature, making architecture validation easier; (4) Our documentation-focused member's concern for **accessibility** influenced the README structure and example-driven design. These concerns, drawn from our "whole team," ensure our architecture serves diverse stakeholder needs.

### c. Define the Architecture Definition roadmap, approach, and strategy.

**IEEE Outcome:** h

**XP Implementation:** "The quarterly cycle is an expression of the possibility of improving long-term plans in the light of experience. Incremental design puts improvement to work by refining the design of the system."

**Project Implementation:** Our architectural roadmap follows an incremental approach: **Phase 1 (Weeks 1-3)**: Establish the core C extension with basic tracing, validating that we can capture execution flow and variables. **Phase 2 (Weeks 4-6)**: Add the interactive breakpoint layer, introducing `idebug.py` as the user-facing interface. **Phase 3 (Weeks 7-9)**: Implement time-travel features including step-back and trace history. **Phase 4 (Week 10)**: Polish and documentation. This roadmap allows us to defer architectural decisions - for example, we initially planned a JSON trace format but deferred that decision until Week 5 when we had working code and could evaluate whether JSON added value. Our approach embodies incremental design: make decisions in light of experience.

### d. Define architecture evaluation criteria based on stakeholder concerns and key requirements.

**IEEE Outcome:** a, i

**XP Implementation:** "Write a failing automated test before changing any code. Test-first programming addresses many problems at once."

**Project Implementation:** We evaluate architecture through test-driven criteria: (1) **Can we test it?** - Each architectural component must be testable in isolation. The C extension is validated by directly importing and calling `cdebugger.start_trace()`. (2) **Does it support pair programming?** - The separation of concerns (trace capture vs. viewing) allows pairs to work on different layers simultaneously. (3) **Is it maintainable?** - We evaluate architecture changes by asking "if we had to add feature X, how many files would we touch?" This led us to keep breakpoint logic in the C extension rather than split across multiple files. (4) **Does it fail gracefully?** - Architecture must handle errors, validated by `test_crash.py` which ensures the debugger itself doesn't crash when the target program does.

### e. Identify and plan for the necessary enabling systems or services needed to support the Architecture Definition process.

**IEEE Outcome:** j

**XP Implementation:** "Automatically build the whole system and run all of the tests in ten minutes."

**Project Implementation:** Our architecture requires several enabling systems: (1) **Build system** - `setup.py` for C extension compilation; (2) **Version control** - Git repository structure supporting multiple developers; (3) **Test execution environment** - Scripts to run test files with tracing enabled; (4) **Development environment** - GCC compiler, Python 3.9+ with development headers. We documented these in README.md as a checklist. Our "ten-minute build" requirement influenced tool selection - we chose a simple Makefile over complex build systems, and direct C compilation over higher-level frameworks. The enabling systems must be as simple and fast as our architecture itself.

### f. Obtain or acquire access to the enabling systems or services to be used.

**IEEE Outcome:** j

**XP Implementation:** "Sit Together. Develop in an open space big enough for the whole team."

**Project Implementation:** All enabling systems are available in our shared lab environment. We ensured each team member could: (1) Clone the repository and run `make` successfully; (2) Execute the test suite and see colored output; (3) Access compiler documentation and Python C API references. During our Week 1 setup, we discovered one team member's laptop couldn't compile C extensions. Rather than having them work in isolation, we pair-programmed with them using lab machines, embodying XP's "sit together" principle. When compiler versions differed between development machines, we documented the minimum required versions in README.md, making enabling system requirements explicit.

## 2. Develop architecture viewpoints

### a. Select, adapt, or develop viewpoints and model kinds based on stakeholder concerns.

**IEEE Outcome:** b

**XP Implementation:** "Make your workspace about your work. An interested observer should be able to walk into the team space and get a general idea of how the project is going in fifteen seconds."

**Project Implementation:** We developed three primary viewpoints: (1) **Component view** - documented in our README showing the three main components: `cdebugger` (C extension), `idebug.py` (interactive CLI), and `traceviewer` (post-execution analysis); (2) **Data flow view** - sketched on our whiteboard showing how execution traces flow from the target program through the C extension to the trace file and eventually to the viewer; (3) **User interaction view** - demonstrated through our test files which show the sequence of commands a user executes. Rather than formal UML diagrams, we use working code and whiteboard sketches visible in our workspace, embodying XP's principle that the workspace should communicate the project's state.

### b. Establish or identify potential architecture framework(s) to be used in developing models and views.

**IEEE Outcome:** b

**XP Implementation:** "Incremental design suggests that the most effective time to design is in the light of experience."

**Project Implementation:** We adopted a **layered architecture framework**: Layer 1 (CPython tracing API), Layer 2 (our C extension wrapping the API), Layer 3 (Python CLI scripts using the extension), Layer 4 (user interaction). This framework emerged through experience rather than upfront selection. Initially, we attempted a single-layer Python-only implementation, but performance concerns led us to introduce the C layer. The framework wasn't selected from a catalog; it evolved as we learned what worked. We document this architecture implicitly through our file organization: C files in the root, Python CLI tools in the root, test files showing usage patterns.

## 3. Develop models and views of candidate architectures

### a. Define the software system context and boundaries in terms of interfaces and interactions with external entities.

**IEEE Outcome:** c

**XP Implementation:** "Develop in an open space big enough for the whole team."

**Project Implementation:** Our system boundaries are clearly defined: **Input boundary** - Python programs to be debugged, specified as command-line arguments to `idebug.py`. **Output boundary** - trace files in `|||`-delimited format, terminal output with ANSI codes. **External interface** - CPython's trace API (`PyEval_SetTrace`), which we wrap but don't control. **User interface** - command-line prompts accepting single-character or word commands. We documented these boundaries in the docstrings of our entry-point files. The context is: our debugger sits between the CPython interpreter and the user, intercepting execution through sanctioned APIs. We do not modify Python's internals or intercept system calls; we work strictly through Python's public C API.

### b. Identify architectural relationships between entities that address key stakeholder concerns and critical software system requirements.

**IEEE Outcome:** a, e

**XP Implementation:** "Pair programming is a dialog between two people simultaneously programming (and analyzing and designing and testing) and trying to program better."

**Project Implementation:** Key architectural relationships include: (1) **C extension ↔ Python interpreter** - The extension registers a trace callback that's invoked on every line execution. This relationship addresses the performance concern: C code runs close to the interpreter's speed. (2) **Trace file ↔ Viewers** - Both `idebug.py` (during execution) and `traceviewer` (post-execution) read the same trace format. This relationship addresses the concern for multiple analysis modes. (3) **Breakpoint store ↔ Trace callback** - Breakpoints are stored in a linked list checked on each trace event. This relationship addresses the concern for pause-on-condition capability. We discovered these relationships through pair programming, where discussing design decisions made relationships explicit.

### c. Allocate concepts, properties, characteristics, behaviors, functions, or constraints that are significant to architecture decisions of the system to architectural entities.

**IEEE Outcome:** e

**XP Implementation:** "Incremental design suggests that the most effective time to design is in the light of experience."

**Project Implementation:** We allocated responsibilities based on emerging understanding: **C extension responsibilities** - Trace capture, breakpoint checking, file I/O for trace writing, memory management. We allocated these to C because they're performance-critical. **Python CLI responsibilities** - User interaction, command parsing, display formatting, file path resolution. We allocated these to Python because they benefit from Python's string handling and we iterate on UX frequently. **Test file responsibilities** - Usage demonstration, regression prevention, executable documentation. We explicitly did not allocate "coverage metrics" or "performance benchmarking" to test files; they're purely functional demonstrations. This allocation emerged over Weeks 2-4 as we refactored code that was initially all in C.

### d. Select, adapt, or develop models of the candidate architectures of the software system.

**IEEE Outcome:** d

**XP Implementation:** "If small, safe steps are how to design, the next question is where in the system to improve the design."

**Project Implementation:** We modeled three candidate architectures: **Candidate 1 (Weeks 1-2)** - Pure Python debugger using `sys.settrace()`. We prototyped this but found variable capture was unreliable across Python versions. **Candidate 2 (Weeks 3-4)** - C extension for tracing, monolithic Python script for interaction. This worked but became unwieldy as features grew. **Candidate 3 (Weeks 5-9, selected)** - C extension for tracing, separate Python scripts for interactive (`idebug.py`) vs. post-execution (`traceviewer`) use. We selected Candidate 3 after experiencing the limitations of Candidates 1 and 2. The models weren't formal; they were working prototypes we could evaluate through actual use.

### e. Compose views from the models in accordance with identified viewpoints to express how the architecture addresses stakeholder concerns and meets system/software requirements.

**IEEE Outcome:** b, d

**XP Implementation:** "Many teams implement this practice in part by putting story cards on a wall."

**Project Implementation:** We compose architectural understanding through multiple views: (1) **File structure view** - visible in our repository listing: C files, Python scripts, test files clearly grouped. (2) **Data flow view** - documented in README with a simple diagram showing: User → idebug.py → cdebugger → trace.log → traceviewer. (3) **Execution view** - demonstrated by running `python3 idebug.py test_clean_simple.py` and observing the flow from setup through execution to post-mortem analysis. (4) **Dependency view** - implicit in our `setup.py` which shows that the C extension has no Python dependencies, while the CLI tools depend on the extension. These views are lightweight and visible in our workspace, not buried in documentation.

### f. Harmonize the architecture models and views with each other.

**IEEE Outcome:** c, d

**XP Implementation:** "Incremental design puts improvement to work by refining the design of the system."

**Project Implementation:** We harmonize views through continuous refactoring. In Week 6, we noticed our file structure view (many files in root directory) contradicted our logical view (clear separation of concerns). We refactored by keeping core components in root but documenting their relationships clearly in README. Another harmonization: our data flow view showed traces written immediately, but our execution view showed buffering for performance. We harmonized by adding explicit `fflush()` calls in the C extension after each trace write, making actual behavior match documented behavior. Harmonization isn't a one-time activity; we check consistency during each weekly retrospective.

## 4. Relate the architecture to design

### a. Identify software system elements that relate to architectural entities and the nature of these relationships.

**IEEE Outcome:** f

**XP Implementation:** "The simple heuristic I have found helpful is to eliminate duplication. If I have the same logic in two places, I work with the design to understand how I can have only one copy."

**Project Implementation:** Architectural entities map to system elements as follows: **Trace Capture architecture** → `trace_callback()` function in `debugger.c`. **Breakpoint Management architecture** → `Breakpoint` struct and associated functions (`add_breakpoint()`, `check_breakpoint()`). **User Interaction architecture** → `DebuggerCLI` class in `idebug.py`. **Post-Execution Analysis architecture** → `TraceViewer` struct in `traceviewer.c`. We eliminated duplication between `idebug.py` and `traceviewer` - both needed to parse trace files, so we standardized the file format rather than implementing two different parsers. The relationship is direct: one architectural concept ↔ one primary code element.

### b. Define the interfaces and interactions among the software system elements and external entities.

**IEEE Outcome:** d

**XP Implementation:** "Pair programming is a dialog between two people simultaneously programming (and analyzing and designing and testing) and trying to program better."

**Project Implementation:** Key interfaces include: (1) **C Extension → Python Interface** - Functions exposed via `PyMethodDef` array: `start_trace()`, `stop_trace()`, `set_breakpoint()`, `clear_breakpoints()`. (2) **Python CLI → C Extension Interface** - Import `cdebugger` and call methods. Error handling via Python exceptions. (3) **Trace File Interface** - Text file with `|||` delimiters, format: `EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES`. (4) **User → CLI Interface** - Single-character commands (`n`, `b`, `c`) and word commands (`next`, `back`, `continue`). We defined these interfaces through pairing: one person writes the consumer, another the provider, ensuring interfaces are actually usable.

### c. Partition, align and allocate requirements to architectural entities and system elements.

**IEEE Outcome:** i

**XP Implementation:** "I like to break stories into tasks that individuals take responsibility for and estimate."

**Project Implementation:** Requirements are allocated as: **"Capture execution trace" requirement** → Allocated to C extension, implemented in `trace_callback()` and `write_variables()` functions. **"Interactive breakpoint setting" requirement** → Allocated to Python CLI, implemented in `DebuggerCLI.do_break()`. **"Step backwards" requirement** → Allocated to both layers: C extension maintains history buffer, CLI provides 'b' command. **"Cross-file breakpoints" requirement** → Allocated to breakpoint checking logic in C extension with path normalization. Each requirement traces to specific tasks in our story cards, which individuals took responsibility for estimating and implementing. Allocation isn't arbitrary; it follows natural boundaries of responsibility and technology constraints.

## 5. Manage the selected architecture

### a. Formalize the architecture governance approach and specify governance-related roles and responsibilities, accountabilities and authorities related to design, quality, security, and safety.

**IEEE Outcome:** f

**XP Implementation:** "Write all production programs with two people sitting at one machine. Pair programmers: Keep each other on task, Brainstorm refinements to the system, Clarify ideas, Take initiative when their partner is stuck, Hold each other accountable to the team's practices."

**Project Implementation:** Our governance approach is peer-based through pair programming rather than hierarchical. Quality is ensured by pairs holding each other accountable: when implementing the step-back feature, one programmer wrote code while the other continuously reviewed for memory leaks and edge cases. Security concerns (e.g., path traversal in file operations) are caught during pairing. We rotate pairs weekly, distributing architectural knowledge. No single person "owns" the architecture; all four team members can make architectural decisions during pairing sessions, but changes must survive code review from another pair. This distributed governance model aligns with XP's emphasis on collective code ownership.

### b. Obtain explicit acceptance of the architecture by stakeholders.

**IEEE Outcome:** f

**XP Implementation:** "Plan work a week at a time. Have a meeting at the beginning of every week."

**Project Implementation:** We obtain acceptance through working software demonstrations. Each week, we demonstrate the current architecture's capabilities to our instructor (primary stakeholder). In Week 4, we demonstrated the C extension capturing traces but without interactive breakpoints - the instructor accepted this architectural foundation. In Week 6, we demonstrated interactive breakpoints working - acceptance confirmed by instructor comments like "this is exactly what I expected to see." Architecture acceptance isn't a signature on a document; it's stakeholders using the working system and providing feedback. Our classmates (secondary stakeholders) acceptance came from them successfully using our debugger on their own code without needing to understand the internal architecture.

### c. Maintain concordance and completeness of the architectural entities and their architectural characteristics.

**IEEE Outcome:** e

**XP Implementation:** "Invest in the design of the system every day. Strive to make the design of the system an excellent fit for the needs of the system that day."

**Project Implementation:** We maintain architectural consistency through daily design investment. During Week 7, we noticed the trace history feature (in C) was inconsistent with the post-execution viewer (separate C program) - both maintained their own data structures for trace entries. We refactored to share a common `TraceEntry` struct definition between files. Another example: when we added colored output to `idebug.py`, we propagated the same color scheme to `traceviewer.c` to maintain consistency. We check architectural completeness during weekly reviews: "Does every story card we completed have a clear home in our architecture?" This daily attention prevents architectural drift.

### d. Organize, assess and control evolution of the architecture models and views to help ensure that the architectural intent is met and the architectural vision and key concepts are correctly implemented.

**IEEE Outcome:** c, e

**XP Implementation:** "The quarterly cycle is an expression of the possibility of improving long-term plans in the light of experience."

**Project Implementation:** Architecture evolution is controlled through our quarterly cycle. At Week 5 (mid-quarter), we assessed whether our three-layer architecture (C extension / Python CLI / User) was meeting our intent of "minimal overhead, maximum usability." We measured: trace overhead (acceptable), command response time (good), user feedback (positive but wanted more features). This assessment led us to extend the architecture with the post-execution viewer rather than cramming more features into the interactive CLI. We control evolution by asking: "Does this change align with our architectural principles?" For example, a suggestion to add a GUI was rejected because it violated our "command-line native" architectural vision. Evolution is intentional, not accidental.

### e. Maintain the architecture definition and evaluation strategy.

**IEEE Outcome:** h

**XP Implementation:** "One feature of XP-style planning is that stories are estimated very early in their life. This gets everyone thinking about how to get the greatest return from the smallest investment."

**Project Implementation:** Our architecture evaluation strategy remains constant: (1) Can we implement stories with reasonable effort? (2) Does the architecture support the "ten-minute build"? (3) Can new team members understand the architecture quickly? We evaluate these criteria weekly. When we added the post-execution viewer in Week 7, it took 3 days to implement - reasonable effort. The build still completes in under 10 minutes. A new team member (simulated by having someone who worked on other stories for two weeks) could understand and modify the viewer in one pairing session. The strategy itself is maintained through our weekly retrospectives where we ask: "Are our evaluation criteria still relevant?" So far, they have been.

### f. Maintain traceability of the architecture.

**IEEE Outcome:** k

**XP Implementation:** "Give stories short names in addition to a short prose or graphical description. Write the stories on index cards and put them on a frequently-passed wall."

**Project Implementation:** Architecture traceability is maintained through: (1) **Story cards to code** - Each card references the files it affects. "Step Back" card lists `debugger.c` (trace history array), `idebug.py` (command handling), `traceviewer.c` (reverse navigation). (2) **Requirements to architecture** - Our README maps high-level requirements to architectural components. (3) **Tests to architecture** - Each test file exercises specific architectural elements: `test_clean_loops.py` validates the trace callback architecture, `test_crash.py` validates error handling architecture. (4) **Commits to decisions** - Git commit messages explain architectural choices: "Separate viewer from CLI for independent development." This traceability is lightweight and visible, not buried in formal documentation.

### g. Provide key artifacts and information items that have been selected for baselines.

**IEEE Outcome:** h, j

**XP Implementation:** "In any plan, include some minor tasks that can be dropped if you get behind."

**Project Implementation:** Architectural baselines are marked by git tags and working test suites: **Baseline 1 (Week 3)**: Core tracing architecture. Artifacts: `debugger.c` with trace callback, `test.py` passing. **Baseline 2 (Week 6)**: Interactive debugging architecture. Artifacts: `idebug.py` with CLI, breakpoint management in C extension, `test_clean_simple.py` passing. **Baseline 3 (Week 9)**: Complete time-travel architecture. Artifacts: All components, all tests passing. Each baseline is deployable and usable. The "slack" in our plan means we can ship any baseline if needed - Baseline 2 is a fully functional debugger, even without time-travel. This flexibility ensures we always have working software demonstrating our architecture.

# 6.4.5 Design Definition process

## 1. Prepare for software system design definition

### a. Map software system elements and architectural entities to design characteristics.

**IEEE Outcome:** a

**XP Implementation:** "Invest in the design of the system every day. Strive to make the design of the system an excellent fit for the needs of the system that day."

**Project Implementation:** We map elements to design characteristics explicitly: **C extension (`debugger.c`)** - Design characteristics: high performance, memory safety, minimal allocations during tracing. **Python CLI (`idebug.py`)** - Design characteristics: user-friendly, forgiving input parsing, helpful error messages. **Trace file format** - Design characteristics: human-readable, parseable, compact. **Breakpoint matching** - Design characteristics: flexible (supports basename and full paths), predictable. These mappings guide daily design decisions. When adding color support, we mapped it to the CLI's "user-friendly" characteristic, not to the C extension's "high performance" characteristic. Design investment happens daily during pair programming where we ask: "What characteristic matters here?"

### b. Define principles for the software system design and evolution.

**IEEE Outcome:** a

**XP Implementation:** "The simple heuristic I have found helpful is to eliminate duplication. If I have the same logic in two places, I work with the design to understand how I can have only one copy."

**Project Implementation:** Our design principles: (1) **Eliminate duplication** - Trace file format is defined once, used everywhere. Breakpoint structure is shared between runtime and history. (2) **Performance over elegance in hot paths** - The trace callback uses raw pointers and manual memory management because it's called thousands of times per second. (3) **Clarity over cleverness in user-facing code** - CLI command handling uses simple if-else chains, not dispatch tables. (4) **Fail visibly** - Errors print messages and exit rather than silently corrupting traces. (5) **Design for testability** - Functions that perform I/O are separated from logic that processes data. These principles evolved through experience and are reinforced during code reviews.

### c. Assess alternatives for obtaining software system elements.

**IEEE Outcome:** b

**XP Implementation:** "One feature of XP-style planning is that stories are estimated very early in their life."

**Project Implementation:** We assessed alternatives for key elements: **Trace capture** - Alternatives: (1) Use `sys.settrace()` in pure Python, (2) Write C extension calling `PyEval_SetTrace()`, (3) Use existing profiling tools like cProfile. We chose (2) for performance and control. **Trace storage** - Alternatives: (1) JSON format, (2) SQLite database, (3) Custom text format. We chose (3) for simplicity and human-readability. **CLI framework** - Alternatives: (1) Click library, (2) cmd.Cmd standard library, (3) Raw input parsing. We chose (2) for balance of features and simplicity. **Viewer implementation** - Alternatives: (1) Python script, (2) C program. We chose (2) to demonstrate C skills and avoid Python startup overhead. Each alternative was estimated for effort, and we chose the option with best value/cost ratio.

### d. Choose the preferred alternative among candidate design solutions for the software system elements.

**IEEE Outcome:** e

**XP Implementation:** "If someone asks me whether I want the Ferrari or the minivan, I choose the Ferrari."

**Project Implementation:** We chose alternatives based on immediate value and constraints: **C extension over pure Python** - Chosen because performance matters for tracing. **Custom text format over JSON** - Chosen because it's simpler to implement and debug. We can visually inspect trace files with `less`. **cmd.Cmd over Click** - Chosen because cmd.Cmd is in stdlib, avoiding external dependencies. **Separate viewer over unified tool** - Chosen because it allows post-mortem analysis without rerunning programs. The choice principle: prefer the solution that delivers value today while keeping future options open. We explicitly deferred choices about remote debugging protocols and multi-process tracing because those features aren't in current stories.

## 2. Establish designs related to each software system element

### a. Transform architectural and design characteristics into the design of software system elements.

**IEEE Outcome:** a

**XP Implementation:** "Incremental design suggests that the most effective time to design is in the light of experience."

**Project Implementation:** We transform characteristics through iterative implementation: **High performance characteristic** → Design decisions: Trace callback minimizes function calls, uses stack-allocated buffers where possible, writes directly to file descriptor. **User-friendly characteristic** → Design decisions: Single-letter command aliases, colored output with ANSI codes, clear error messages like "Breakpoint 3 doesn't exist" not "Index out of bounds." **Memory safety characteristic** → Design decisions: Free allocated memory on shutdown, null-check all pointer dereferences, limit trace history to MAX_TRACE_HISTORY entries. These transformations happened as we gained experience with what works. Initial design lacked command aliases; we added them after watching users type "next" repeatedly.

### b. Define and prepare or obtain the necessary design enablers.

**IEEE Outcome:** c

**XP Implementation:** "Write all production programs with two people sitting at one machine."

**Project Implementation:** Design enablers we prepared: (1) **Python C API reference materials** - Bookmarked and printed relevant sections of Python/C API documentation. (2) **Example C extensions** - Studied existing extensions to learn patterns for memory management and exception handling. (3) **Terminal capability detection** - Researched ANSI escape codes for cross-platform colored output. (4) **Test infrastructure** - Created template test files that others could copy when adding new test cases. (5) **Debugging tools** - Configured GDB to work with Python C extensions. We obtained these enablers through pair programming - one person researches while the other experiments, then we swap. This ensures knowledge is shared.

### c. Examine design alternatives and feasibility of implementation.

**IEEE Outcome:** e

**XP Implementation:** "Pairing doesn't mean that you can't think alone. People need both companionship and privacy. If you need to work on an idea alone, go do it."

**Project Implementation:** We examined alternatives through rapid prototyping. For the step-back feature, we prototyped three designs: (1) **Rewind execution** - Let Python interpreter run backwards. Feasibility: impossible without modifying CPython. (2) **Checkpoint and restore** - Save program state at each step. Feasibility: complex, high memory usage. (3) **Replay from history** - Store execution history, display previous entries. Feasibility: high, implemented. For variable capture, we prototyped accessing `f_locals` directly vs. calling `PyFrame_FastToLocals()`. The second approach worked more reliably across Python versions. Prototyping happened during solo exploration time, with findings shared during pairing sessions. We valued feasibility over theoretical elegance.

### d. Refine or define the interfaces among the software system elements and with external entities.

**IEEE Outcome:** d

**XP Implementation:** "Pair programming is a dialog between two people simultaneously programming."

**Project Implementation:** Interface refinement examples: **C Extension Interface** - Initially: `start_trace(filename)` returns void. Refined: returns int (success/failure) so Python code can handle errors. **Breakpoint Interface** - Initially: `set_breakpoint(filename, lineno)`. Refined: added return value to indicate success, allowing CLI to confirm to user. **Trace File Interface** - Initially: variables stored as `name:value` pairs. Refined: variables stored as `name=value` for easier parsing with Python's string.split('='). **Command Interface** - Initially: only full words ("next", "back"). Refined: added single-letter shortcuts ("n", "b") for efficiency. Refinements emerged from using our own interfaces during development. Pairing ensured both the provider and consumer perspectives informed interface design.

### e. Establish the design artifacts.

**IEEE Outcome:** f

**XP Implementation:** "Write a failing automated test before changing any code."

**Project Implementation:** Our design artifacts are primarily code and tests: (1) **Code files** - `debugger.c`, `idebug.py`, `traceviewer.c` with inline comments explaining design decisions. (2) **Test files** - `test_clean_simple.py`, `test_clean_loops.py`, `test_crash.py` demonstrating design through usage. (3) **README.md** - Documents design at a high level with examples. (4) **Makefile** - Codifies build design and dependencies. (5) **Example trace files** - Generated by running tests, showing trace format design. We deliberately avoid separate design documents. The artifacts are executable and testable. When someone asks "How is variable capture designed?", we point them to `test_variables.py` and the `write_variables()` function, not to a Word document.

## 3. Assess alternatives for obtaining software system elements

### a. Determine technologies required for each element composing the software system.

**IEEE Outcome:** b

**XP Implementation:** "Automatically build the whole system and run all of the tests in ten minutes."

**Project Implementation:** Technologies determined per element: **C extension** - Requires: GCC or Clang, Python development headers (python3-dev), Python C API knowledge. **Python CLI** - Requires: Python 3.9+, cmd module (stdlib), readline for history (optional). **Trace viewer** - Requires: C compiler, optionally readline library for better input handling. **Build system** - Requires: setuptools (for C extension), Make (for trace viewer). **Testing** - Requires: ability to run Python programs with modified sys.path. We documented technology requirements in README with installation commands for Ubuntu and macOS. All technologies support our "ten-minute build" goal - installation and initial build complete in under 10 minutes on a fresh system.

### b. Identify candidate alternatives for the software system elements.

**IEEE Outcome:** e

**XP Implementation:** "One feature of XP-style planning is that stories are estimated very early in their life."

**Project Implementation:** Candidate alternatives we identified: **For trace storage**: (1) In-memory only (fast but limited), (2) File-based (slower but unlimited), (3) Database (complex but queryable). **For CLI**: (1) Pure Python (simplest), (2) C-based (fastest). **For variable representation**: (1) repr() output (readable), (2) JSON serialization (structured), (3) Pickle (complete but opaque). **For breakpoint specification**: (1) Line numbers only, (2) File:line pairs, (3) Function names. We estimated each alternative early in planning. For example, database storage estimated at 2 weeks vs. file-based at 3 days. Simple estimation helped us choose efficiently.

### c. Assess each candidate alternative against criteria developed from expected design characteristics and element requirements to determine suitability for the intended application.

**IEEE Outcome:** e

**XP Implementation:** "The cycle is to do the best you can today, striving for the awareness and understanding necessary to do better tomorrow."

**Project Implementation:** Assessment criteria and outcomes: **Trace storage alternatives** - Criteria: speed, simplicity, debuggability. File-based scored highest: adequate speed, simple implementation, human-readable output for debugging. **CLI alternatives** - Criteria: development speed, user familiarity. Python-based scored highest: rapid iteration, developers familiar with Python CLI tools. **Variable representation** - Criteria: readability, completeness. repr() scored highest: shows values clearly, handles most Python types. **Breakpoint specification** - Criteria: flexibility, ease of use. File:line scored highest: unambiguous, supports multiple files. We didn't seek perfect solutions; we sought "best we can do today" solutions that we could improve tomorrow if needed.

### d. Choose the preferred alternative among candidate design solutions for the software system elements.

**IEEE Outcome:** e

**XP Implementation:** "However, as soon as someone says, 'Do you want the Ferrari for $150,000 or the minivan for $25,000?' I can begin to make an informed decision."

**Project Implementation:** Chosen alternatives with rationale: **File-based trace storage** - Chosen over database ($25,000 minivan) despite database's superior queryability ($150,000 Ferrari). Rationale: We need basic storage now, can add database later if needed. **Python CLI** - Chosen over C CLI. Rationale: Faster to develop and modify during iteration. **repr() for variables** - Chosen over JSON serialization. Rationale: Simpler, handles arbitrary Python objects. **File:line breakpoints** - Chosen over function-name breakpoints. Rationale: Line numbers are unambiguous and match how developers think about code. Choices prioritize delivering value quickly while maintaining quality. We explicitly accepted that we might need to change these choices later based on experience.

## 4. Manage the design

### a. Capture the design and rationale.

**IEEE Outcome:** a

**XP Implementation:** "Invest in the design of the system every day."

**Project Implementation:** We capture design through: (1) **Code comments** - The C extension includes comments like "// Use raw file I/O for performance, avoid Python overhead" explaining design rationale. (2) **Commit messages** - Git commits explain why: "Switch from JSON to delimited format - simpler parsing, human readable." (3) **README sections** - Design decisions section documents "Why C for tracing?" with performance rationale. (4) **Test file docstrings** - Tests document intended behavior: "This test validates that breakpoints persist across loop iterations." (5) **Pair programming conversations** - Design discussions happen live and knowledge transfers to both programmers. Rationale is captured where it's most useful - near the code, not in separate documents.

### b. Establish traceability between the detailed design elements, the system/software requirements, and the architectural entities of the software system architecture.

**IEEE Outcome:** h

**XP Implementation:** "Give stories short names in addition to a short prose or graphical description. Write the stories on index cards and put them on a frequently-passed wall."

**Project Implementation:** Traceability established through: **Story card "Loop Breakpoints"** → traces to requirement "Debug loops effectively" → traces to architectural entity "Trace Callback" → traces to design element `check_breakpoint()` in `debugger.c` → validated by `test_clean_loops.py`. **Story card "Step Back"** → traces to requirement "Time-travel debugging" → traces to architectural entity "Trace History" → traces to design elements `trace_history[]` array and `show_trace_entry()` → validated by manual testing. **Story card "Cross-File Debug"** → traces to requirement "Support imports" → traces to architectural entity "Path Matching" → traces to design element `filenames_match()` → validated by `test_import_main.py`. Traceability is visible: story cards on wall reference test files; test files exercise specific functions; functions implement architectural concepts.

### c. Determine the status of the software system and element design.

**IEEE Outcome:** a, e

**XP Implementation:** "Plan work a week at a time. Have a meeting at the beginning of every week."

**Project Implementation:** Design status determined weekly: **Week 3 status**: Trace capture design complete and stable. Breakpoint design prototyped but unstable. CLI design not started. **Week 6 status**: All three designs (trace capture, breakpoints, CLI) complete and stable. Time-travel design in progress. **Week 9 status**: All designs complete, undergoing refinement. We determine status by running tests and by attempting to implement next week's stories. If implementing a story requires redesign of existing elements, status is "unstable." If stories build cleanly on existing design, status is "stable." Weekly planning meetings make status explicit: "Can we implement this week's stories with current design?"

### d. Provide key artifacts and information items that have been selected for baselines.

**IEEE Outcome:** f, g

**XP Implementation:** "In any plan, include some minor tasks that can be dropped if you get behind."

**Project Implementation:** Design baseline artifacts: **Baseline 1 (Week 3)** - `debugger.c` trace callback design, basic trace file format, setup.py build configuration. **Baseline 2 (Week 6)** - Complete C extension with breakpoints, `idebug.py` CLI design, test file patterns. **Baseline 3 (Week 9)** - All designs including time-travel features, complete test suite, documentation. Each baseline is tagged in git and includes: source code, tests passing, README section documenting the design. "Slack" features like advanced trace viewers or conditional breakpoints are documented as future design directions but not part of baselines. This ensures baselines represent working, deployable designs.
