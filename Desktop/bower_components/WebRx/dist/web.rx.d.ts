/// <reference path="../node_modules/rx/ts/rx.all.d.ts" />
/// <reference path="../test/typings/jquery.d.ts" />
declare module wx {
    function createWeakMap<TKey, T>(disableNativeSupport?: boolean): IWeakMap<TKey, T>;
}
declare module wx.res {
    var injector: string;
    var domManager: string;
    var router: string;
    var messageBus: string;
    var expressionCompiler: string;
    var htmlTemplateEngine: string;
    var hasValueBindingValue: string;
    var valueBindingValue: string;
}
declare module wx {
    interface IInjector {
        register(key: string, factory: Array<any>, singleton?: boolean): IInjector;
        register(key: string, factory: () => any, singleton?: boolean): IInjector;
        register(key: string, instance: any): IInjector;
        get<T>(key: string, args?: any): T;
        resolve<T>(iaa: Array<any>, args?: any): T;
    }
    interface IWeakMap<TKey extends Object, T> {
        set(key: TKey, value: T): void;
        get(key: TKey): T;
        has(key: TKey): boolean;
        delete(key: TKey): void;
        isEmulated: boolean;
    }
    interface ISet<T> {
        add(value: T): ISet<T>;
        has(key: T): boolean;
        delete(key: T): boolean;
        clear(): void;
        forEach(callback: (T) => void, thisArg?: any): void;
        size: number;
        isEmulated: boolean;
    }
    interface IObservableProperty<T> extends Rx.IDisposable {
        (newValue: T): void;
        (): T;
        changing: Rx.Observable<T>;
        changed: Rx.Observable<T>;
        source?: Rx.Observable<T>;
    }
    interface IPropertyChangedEventArgs {
        sender: any;
        propertyName: string;
    }
    interface IListChangeInfo<T> {
        items: T[];
        from: number;
        to?: number;
    }
    interface INotifyListItemChanged {
        itemChanging: Rx.Observable<IPropertyChangedEventArgs>;
        itemChanged: Rx.Observable<IPropertyChangedEventArgs>;
        changeTrackingEnabled: boolean;
    }
    interface INotifyListChanged<T> {
        listChanging: Rx.Observable<boolean>;
        listChanged: Rx.Observable<boolean>;
        itemsAdded: Rx.Observable<IListChangeInfo<T>>;
        beforeItemsAdded: Rx.Observable<IListChangeInfo<T>>;
        itemsRemoved: Rx.Observable<IListChangeInfo<T>>;
        beforeItemsRemoved: Rx.Observable<IListChangeInfo<T>>;
        beforeItemsMoved: Rx.Observable<IListChangeInfo<T>>;
        itemsMoved: Rx.Observable<IListChangeInfo<T>>;
        beforeItemReplaced: Rx.Observable<IListChangeInfo<T>>;
        itemReplaced: Rx.Observable<IListChangeInfo<T>>;
        lengthChanging: Rx.Observable<number>;
        lengthChanged: Rx.Observable<number>;
        isEmptyChanged: Rx.Observable<boolean>;
        shouldReset: Rx.Observable<any>;
        suppressChangeNotifications(): Rx.IDisposable;
    }
    interface IObservableReadOnlyList<T> extends INotifyListChanged<T>, INotifyListItemChanged {
        length: IObservableProperty<number>;
        get(index: number): T;
        isReadOnly: boolean;
        toArray(): Array<T>;
        project<TNew>(filter?: (item: T) => boolean, orderer?: (a: TNew, b: TNew) => number, selector?: (T) => TNew, scheduler?: Rx.IScheduler): IObservableReadOnlyList<TNew>;
    }
    interface IObservableList<T> extends IObservableReadOnlyList<T> {
        isEmpty: IObservableProperty<boolean>;
        set(index: number, item: T): any;
        add(item: T): void;
        push(item: T): void;
        clear(): void;
        contains(item: T): boolean;
        remove(item: T): boolean;
        indexOf(item: T): number;
        insert(index: number, item: T): void;
        removeAt(index: number): void;
        addRange(collection: Array<T>): void;
        insertRange(index: number, collection: Array<T>): void;
        move(oldIndex: any, newIndex: any): void;
        removeAll(items: Array<T>): void;
        removeRange(index: number, count: number): void;
        reset(): void;
        sort(comparison: (a: T, b: T) => number): void;
        forEach(callbackfn: (value: T, index: number, array: T[]) => void, thisArg?: any): void;
        map<U>(callbackfn: (value: T, index: number, array: T[]) => U, thisArg?: any): U[];
        filter(callbackfn: (value: T, index: number, array: T[]) => boolean, thisArg?: any): T[];
        every(callbackfn: (value: T, index: number, array: T[]) => boolean, thisArg?: any): boolean;
        some(callbackfn: (value: T, index: number, array: T[]) => boolean, thisArg?: any): boolean;
    }
    interface IHandleObservableErrors {
        thrownExceptions: Rx.Observable<Error>;
    }
    interface ICommand<T> extends Rx.IDisposable, IHandleObservableErrors {
        canExecute(parameter: any): boolean;
        execute(parameter: any): void;
        canExecuteObservable: Rx.Observable<boolean>;
        isExecuting: Rx.Observable<boolean>;
        results: Rx.Observable<T>;
        executeAsync(parameter?: any): Rx.Observable<T>;
    }
    interface IDataContext {
        $data: any;
        $root: any;
        $parent: any;
        $parents: any[];
    }
    interface INodeState {
        cleanup: Rx.CompositeDisposable;
        isBound: boolean;
        model?: any;
    }
    interface IObjectLiteralToken {
        key?: string;
        unknown?: string;
        value?: string;
    }
    interface IExpressionFilter {
        (...args: Array<any>): any;
    }
    interface IExpressionCompilerOptions {
        disallowFunctionCalls?: boolean;
        filters?: {
            [filterName: string]: IExpressionFilter;
        };
    }
    interface ICompiledExpression {
        (scope?: any, locals?: any): any;
        literal?: boolean;
        constant?: boolean;
        assign?: (self, value, locals) => any;
    }
    interface ICompiledExpressionRuntimeHooks {
        readFieldHook?: (o: any, field: any) => any;
        writeFieldHook?: (o: any, field: any, newValue: any) => any;
        readIndexHook?: (o: any, field: any) => any;
        writeIndexHook?: (o: any, field: any, newValue: any) => any;
    }
    interface IExpressionCompiler {
        compileExpression(src: string, options?: IExpressionCompilerOptions, cache?: {
            [exp: string]: ICompiledExpression;
        }): ICompiledExpression;
        getRuntimeHooks(locals: any): ICompiledExpressionRuntimeHooks;
        setRuntimeHooks(locals: any, hooks: ICompiledExpressionRuntimeHooks): void;
        parseObjectLiteral(objectLiteralString: any): Array<IObjectLiteralToken>;
    }
    interface IAnimation {
        prepare(element: Node | Array<Node> | HTMLElement | Array<HTMLElement> | NodeList, params?: any): void;
        run(element: Node | Array<Node> | HTMLElement | Array<HTMLElement> | NodeList, params?: any): Rx.Observable<any>;
        complete(element: Node | Array<Node> | HTMLElement | Array<HTMLElement> | NodeList, params?: any): void;
    }
    interface IDomManager {
        applyBindings(model: any, rootNode: Node): void;
        applyBindingsToDescendants(ctx: IDataContext, rootNode: Node): void;
        cleanNode(rootNode: Node): void;
        cleanDescendants(rootNode: Node): void;
        setNodeState(node: Node, state: INodeState): void;
        getDataContext(node: Node): IDataContext;
        getNodeState(node: Node): INodeState;
        createNodeState(model?: any): INodeState;
        isNodeBound(node: Node): boolean;
        clearNodeState(node: Node): any;
        compileBindingOptions(value: string, module: IModule): any;
        getObjectLiteralTokens(value: string): Array<IObjectLiteralToken>;
        getBindingDefinitions(node: Node): Array<{
            key: string;
            value: string;
        }>;
        registerDataContextExtension(extension: (node: Node, ctx: IDataContext) => void): any;
        evaluateExpression(exp: ICompiledExpression, ctx: IDataContext): any;
        expressionToObservable(exp: ICompiledExpression, ctx: IDataContext, evalObs?: Rx.Observer<any>): Rx.Observable<any>;
    }
    interface IBindingHandler {
        applyBinding(node: Node, options: string, ctx: IDataContext, state: INodeState, module: IModule): void;
        configure(options: any): void;
        priority: number;
        controlsDescendants?: boolean;
    }
    interface IBindingRegistry {
        binding(name: string, handler: IBindingHandler): IBindingRegistry;
        binding(name: string, handler: string): IBindingRegistry;
        binding(names: string[], handler: IBindingHandler): IBindingRegistry;
        binding(names: string[], handler: string): IBindingRegistry;
        binding(name: string): IBindingHandler;
    }
    interface IComponentTemplateDescriptor {
        (params?: any): string | Node[];
        require?: string;
        promise?: Rx.IPromise<Node[]>;
        resolve?: string;
        element?: string | Node;
    }
    interface IComponentViewModelDescriptor {
        (params: any): any;
        require?: string;
        promise?: Rx.IPromise<string>;
        resolve?: string;
        instance?: any;
    }
    interface IComponentDescriptor {
        require?: string;
        resolve?: string;
        template?: string | Node[] | IComponentTemplateDescriptor;
        viewModel?: Array<any> | IComponentViewModelDescriptor;
        preBindingInit?: string;
        postBindingInit?: string;
    }
    interface IComponent {
        template: Node[];
        viewModel?: any;
        preBindingInit?: string;
        postBindingInit?: string;
    }
    interface IComponentRegistry {
        component(name: string, descriptor: IComponentDescriptor): IComponentRegistry;
        hasComponent(name: string): boolean;
        loadComponent(name: string, params?: Object): Rx.Observable<IComponent>;
    }
    interface IExpressionFilterRegistry {
        filter(name: string, filter: IExpressionFilter): IExpressionFilterRegistry;
        filter(name: string): IExpressionFilter;
        filters(): {
            [filterName: string]: IExpressionFilter;
        };
    }
    interface IAnimationRegistry {
        animation(name: string, filter: IAnimation): IAnimationRegistry;
        animation(name: string): IAnimation;
    }
    interface IModuleDescriptor {
        (module: IModule): void;
        require?: string;
        promise?: Rx.IPromise<string>;
        resolve?: string;
        instance?: any;
    }
    interface IModule extends IComponentRegistry, IBindingRegistry, IExpressionFilterRegistry, IAnimationRegistry {
        name: string;
        merge(other: IModule): IModule;
    }
    interface ITemplateEngine {
        parse(templateSource: string): Node[];
    }
    interface IWebRxApp extends IModule {
        defaultExceptionHandler: Rx.Observer<Error>;
        mainThreadScheduler: Rx.IScheduler;
        templateEngine: ITemplateEngine;
        history: IHistory;
        title: IObservableProperty<string>;
    }
    interface IRoute {
        parse(url: any): Object;
        stringify(params?: Object): string;
        concat(route: IRoute): IRoute;
        isAbsolute: boolean;
        params: Array<string>;
    }
    interface IViewAnimationDescriptor {
        enter?: string | IAnimation;
        leave?: string | IAnimation;
    }
    interface IRouterStateConfig {
        name: string;
        route?: string | IRoute;
        views?: {
            [view: string]: string | {
                component: string;
                params?: any;
                animations?: IViewAnimationDescriptor;
            };
        };
        params?: any;
        onEnter?: (config: IRouterStateConfig, params?: any) => void;
        onLeave?: (config: IRouterStateConfig, params?: any) => void;
    }
    interface IRouterState {
        name: string;
        uri: string;
        params: any;
        views: {
            [view: string]: string | {
                component: string;
                params?: any;
                animations?: IViewAnimationDescriptor;
            };
        };
        onEnter?: (config: IRouterStateConfig, params?: any) => void;
        onLeave?: (config: IRouterStateConfig, params?: any) => void;
    }
    interface IViewConfig {
        component: string;
        params?: any;
        animations?: IViewAnimationDescriptor;
    }
    const enum RouterLocationChangeMode {
        add = 1,
        replace = 2,
    }
    interface IStateChangeOptions {
        location?: boolean | RouterLocationChangeMode;
        force?: boolean;
    }
    interface IHistory {
        onPopState: Rx.Observable<PopStateEvent>;
        location: Location;
        length: number;
        state: any;
        back(): void;
        forward(): void;
        replaceState(statedata: any, title: string, url?: string): void;
        pushState(statedata: any, title: string, url?: string): void;
    }
    interface IRouter {
        state(config: IRouterStateConfig): IRouter;
        current: IObservableProperty<IRouterState>;
        updateCurrentStateParams(withParamsAction: (params: any) => void): void;
        go(to: string, params?: Object, options?: IStateChangeOptions): void;
        uri(state: string, params?: {}): string;
        reload(): void;
        get(state: string): IRouterStateConfig;
        is(state: string, params?: any, options?: any): any;
        includes(state: string, params?: any, options?: any): any;
        reset(): void;
        getViewComponent(viewName: string): IViewConfig;
    }
    interface IMessageBus {
        registerScheduler(scheduler: Rx.IScheduler, contract: string): void;
        listen<T>(contract: string): Rx.IObservable<T>;
        isRegistered(contract: string): boolean;
        registerMessageSource<T>(source: Rx.Observable<T>, contract: string): Rx.IDisposable;
        sendMessage<T>(message: T, contract: string): void;
    }
}
declare module Rx {
    interface Observable<T> extends IObservable<T> {
        toProperty(initialValue?: T): wx.IObservableProperty<T>;
        continueWith(action: () => void): Observable<any>;
        continueWith<TResult>(action: (T) => TResult): Observable<TResult>;
        continueWith<TOther>(obs: Rx.Observable<TOther>): Observable<TOther>;
    }
    interface ObservableStatic {
        startDeferred<T>(action: () => T): Rx.Observable<T>;
    }
}
declare module wx {
    interface IUnknown {
        queryInterface(iid: string): boolean;
    }
}
declare module wx.internal {
    class PropertyChangedEventArgs implements IPropertyChangedEventArgs {
        constructor(sender: any, propertyName: string);
        sender: any;
        propertyName: string;
    }
}
declare module wx {
    var noop: () => void;
    function isStrictMode(): boolean;
    function isPrimitive(target: any): boolean;
    function isProperty(target: any): boolean;
    function isCommand(target: any): boolean;
    function isList(target: any): boolean;
    function isRxScheduler(target: any): boolean;
    function isRxObservable(target: any): boolean;
    function unwrapProperty(prop: any): any;
    function isInUnitTest(): boolean;
    function args2Array(args: IArguments): Array<any>;
    function formatString(fmt: string, ...args: any[]): string;
    function trimString(str: string): string;
    function extend(src: Object, dst: Object, inherited?: boolean): Object;
    class PropertyInfo<T> {
        constructor(propertyName: string, property: T);
        propertyName: string;
        property: T;
    }
    function queryInterface(target: any, iid: string): boolean;
    function supportsQueryInterface(target: any): boolean;
    function getOwnPropertiesImplementingInterface<T>(target: any, iid: string): PropertyInfo<T>[];
    function getOid(o: any): string;
    function toggleCssClass(node: HTMLElement, shouldHaveClass: boolean, ...classNames: string[]): void;
    function triggerReflow(el: HTMLElement): void;
    function isFunction(obj: any): boolean;
    function isDisposable(obj: any): boolean;
    function isEqual(a: any, b: any, aStack?: any, bStack?: any): boolean;
    function cloneNodeArray(nodes: Array<Node>): Array<Node>;
    function nodeListToArray(nodes: NodeList): Node[];
    function nodeChildrenToArray<T>(node: Node): T[];
    function using<T extends Rx.Disposable>(disp: T, action: (disp?: T) => void): void;
    function observableRequire<T>(module: string): Rx.Observable<T>;
    function observeObject(target: any, onChanging?: boolean): Rx.Observable<IPropertyChangedEventArgs>;
    function whenAny<TRet, T1>(property1: IObservableProperty<T1>, selector: (T1) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, selector: (T1, T2, T3, T4, T5) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, selector: (T1, T2, T3, T4, T5) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3, T4>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, property4: IObservableProperty<T4>, selector: (T1, T2, T3, T4, T5) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3, T4, T5>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, property4: IObservableProperty<T4>, property5: IObservableProperty<T5>, selector: (T1, T2, T3, T4, T5) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3, T4, T5, T6>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, property4: IObservableProperty<T4>, property5: IObservableProperty<T5>, property6: IObservableProperty<T6>, selector: (T1, T2, T3, T4, T5, T6) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3, T4, T5, T6, T7>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, property4: IObservableProperty<T4>, property5: IObservableProperty<T5>, property6: IObservableProperty<T6>, property7: IObservableProperty<T7>, selector: (T1, T2, T3, T4, T5, T6, T7) => TRet): Rx.Observable<TRet>;
    function whenAny<TRet, T1, T2, T3, T4, T5, T6, T7, T8>(property1: IObservableProperty<T1>, property2: IObservableProperty<T2>, property3: IObservableProperty<T3>, property4: IObservableProperty<T4>, property5: IObservableProperty<T5>, property6: IObservableProperty<T6>, property7: IObservableProperty<T7>, property8: IObservableProperty<T8>, selector: (T1, T2, T3, T4, T5, T6, T7, T8) => TRet): Rx.Observable<TRet>;
    module internal {
        function throwError(fmt: string, ...args: any[]): void;
        function emitError(fmt: string, ...args: any[]): void;
    }
}
declare module wx {
    var injector: IInjector;
}
declare module wx {
    function createSet<T>(disableNativeSupport?: boolean): ISet<T>;
    function setToArray<T>(src: ISet<T>): Array<T>;
}
declare module wx.env {
    interface IBrowserProperties {
        version: number;
    }
    interface IIEBrowserProperties extends IBrowserProperties {
        getSelectionChangeObservable(el: HTMLElement): Rx.Observable<Document>;
    }
    var ie: IIEBrowserProperties;
    var opera: IBrowserProperties;
    var safari: IBrowserProperties;
    var firefox: IBrowserProperties;
    var isSupported: boolean;
    var jQueryInstance: any;
    function cleanExternalData(node: Node): any;
}
declare module wx {
    class IID {
        static IUnknown: string;
        static IDisposable: string;
        static IObservableProperty: string;
        static IReactiveNotifyPropertyChanged: string;
        static IHandleObservableErrors: string;
        static IObservableList: string;
        static IList: string;
        static IReactiveNotifyCollectionChanged: string;
        static IReactiveNotifyCollectionItemChanged: string;
        static IReactiveDerivedList: string;
        static IMoveInfo: string;
        static IObservedChange: string;
        static ICommand: string;
        static IReadOnlyList: string;
    }
}
declare module wx {
    function property<T>(initialValue?: T): IObservableProperty<T>;
}
declare var createMockHistory: () => wx.IHistory;
declare module wx {
    module internal {
        var moduleConstructor: any;
    }
    var app: IWebRxApp;
    function module(name: string, descriptor: Array<any> | IModuleDescriptor): typeof wx;
    function loadModule(name: string): Rx.Observable<IModule>;
}
declare module wx {
    module internal {
        var domManagerConstructor: any;
    }
    function applyBindings(model: any, node?: Node): void;
    function cleanNode(node: Node): void;
}
declare module wx {
    module internal {
        var checkedBindingConstructor: any;
    }
}
declare module wx {
    interface ICommandBindingOptions {
        command: ICommand<any>;
        parameter?: any;
    }
    module internal {
        var commandBindingConstructor: any;
    }
}
declare module wx {
    interface INodeState {
        module?: IModule;
    }
    module internal {
        var moduleBindingConstructor: any;
    }
}
declare module wx {
    interface IComponentBindingOptions {
        name: string;
        params?: Object;
    }
    module internal {
        var componentBindingConstructor: any;
    }
}
declare module wx {
    interface IEventBindingOptions {
        [eventName: string]: (ctx: IDataContext, event: Event) => any | Rx.Observer<Event>;
    }
    module internal {
        var eventBindingConstructor: any;
    }
}
declare module wx.internal {
    class VirtualChildNodes {
        constructor(targetNode: Node, initialSyncToTarget: boolean, insertCB?: (node: Node, callbackData: any) => void, removeCB?: (node: Node) => void);
        appendChilds(nodes: Node[], callbackData?: any): void;
        insertChilds(index: number, nodes: Node[], callbackData?: any): void;
        removeChilds(index: number, count: number, keepDom: boolean): Node[];
        clear(): void;
        targetNode: Node;
        childNodes: Array<Node>;
        private insertCB;
        private removeCB;
    }
}
declare module wx {
    class RefCountDisposeWrapper implements Rx.IDisposable {
        constructor(inner: Rx.IDisposable, initialRefCount?: number);
        private inner;
        private refCount;
        addRef(): void;
        release(): number;
        dispose(): void;
    }
}
declare module wx {
    interface IForeachAnimationDescriptor {
        itemEnter?: string | IAnimation;
        itemLeave?: string | IAnimation;
    }
    interface IForEachBindingOptions extends IForeachAnimationDescriptor {
        data: any;
        hooks?: IForEachBindingHooks | string;
    }
    interface IForEachBindingHooks {
        afterRender?(nodes: Node[], data: any): void;
        afterAdd?(nodes: Node[], data: any, index: number): void;
        beforeRemove?(nodes: Node[], data: any, index: number): void;
        beforeMove?(nodes: Node[], data: any, index: number): void;
        afterMove?(nodes: Node[], data: any, index: number): void;
    }
    module internal {
        var forEachBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var hasFocusBindingConstructor: any;
    }
}
declare module wx {
    interface IIfAnimationDescriptor {
        enter?: string | IAnimation;
        leave?: string | IAnimation;
    }
    interface IIfBindingOptions extends IIfAnimationDescriptor {
        condition: string;
    }
    module internal {
        var ifBindingConstructor: any;
        var notifBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var cssBindingConstructor: any;
        var attrBindingConstructor: any;
        var styleBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var selectedValueBindingConstructor: any;
    }
}
declare module wx {
    interface IVisibleBindingOptions {
        useCssClass: boolean;
        hiddenClass: string;
    }
    module internal {
        var textBindingConstructor: any;
        var htmlBindingConstructor: any;
        var visibleBindingConstructor: any;
        var hiddenBindingConstructor: any;
        var disableBindingConstructor: any;
        var enableBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var textInputBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        function getNodeValue(node: Node, domManager: IDomManager): any;
        function setNodeValue(node: Node, value: any, domManager: IDomManager): void;
    }
    module internal {
        var valueBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var withBindingConstructor: any;
    }
}
declare module wx {
    class Lazy<T> {
        constructor(createValue: () => T);
        value: T;
        isValueCreated: boolean;
        private createValue;
        private createdValue;
    }
}
declare module wx.internal {
    function createScheduledSubject<T>(scheduler: Rx.IScheduler, defaultObserver?: Rx.Observer<T>, defaultSubject?: Rx.ISubject<T>): Rx.Subject<T>;
}
declare module wx {
}
declare module wx {
    module internal {
        var listConstructor: any;
    }
    function list<T>(initialContents?: Array<T>, resetChangeThreshold?: number, scheduler?: Rx.IScheduler): IObservableList<T>;
}
declare module wx {
    interface IRadioGroupComponentParams {
        items: any;
        groupName?: string;
        itemText?: string;
        itemValue?: string;
        itemClass?: string;
        selectedValue?: any;
        afterRender?(nodes: Node[], data: any): void;
        noCache?: boolean;
    }
    module internal {
        var radioGroupComponentConstructor: any;
    }
}
declare module wx {
    interface ISelectComponentParams {
        name?: string;
        items: any;
        itemText?: string;
        itemValue?: string;
        itemClass?: string;
        multiple?: boolean;
        required?: boolean;
        autofocus?: boolean;
        size?: number;
        selectedValue?: any;
        afterRender?(nodes: Node[], data: any): void;
        noCache?: boolean;
    }
    module internal {
        var selectComponentConstructor: any;
    }
}
declare module wx {
    interface IAnimationCssClassInstruction {
        css: string;
        add: boolean;
        remove: boolean;
    }
    function animation(prepareTransitionClass: string | Array<string> | Array<IAnimationCssClassInstruction>, startTransitionClass: string | Array<string> | Array<IAnimationCssClassInstruction>, completeTransitionClass: string | Array<string> | Array<IAnimationCssClassInstruction>): IAnimation;
    function animation(run: (element: HTMLElement, params?: any) => Rx.Observable<any>, prepare?: (element: HTMLElement, params?: any) => void, complete?: (element: HTMLElement, params?: any) => void): IAnimation;
}
declare module wx {
    module internal {
        var commandConstructor: any;
    }
    function command(execute: (any) => void, canExecute?: Rx.Observable<boolean>, scheduler?: Rx.IScheduler, thisArg?: any): ICommand<any>;
    function command(execute: (any) => void, canExecute?: Rx.Observable<boolean>, thisArg?: any): ICommand<any>;
    function command(execute: (any) => void, thisArg?: any): ICommand<any>;
    function command(canExecute?: Rx.Observable<boolean>, scheduler?: Rx.IScheduler): ICommand<any>;
    function asyncCommand<T>(canExecute: Rx.Observable<boolean>, executeAsync: (any) => Rx.Observable<T>, scheduler?: Rx.IScheduler, thisArg?: any): ICommand<T>;
    function asyncCommand<T>(canExecute: Rx.Observable<boolean>, executeAsync: (any) => Rx.Observable<T>, thisArg?: any): ICommand<T>;
    function asyncCommand<T>(executeAsync: (any) => Rx.Observable<T>, scheduler?: Rx.IScheduler, thisArg?: any): ICommand<T>;
    function asyncCommand<T>(executeAsync: (any) => Rx.Observable<T>, thisArg?: any): ICommand<T>;
    function combinedCommand(canExecute: Rx.Observable<boolean>, ...commands: ICommand<any>[]): ICommand<any>;
    function combinedCommand(...commands: ICommand<any>[]): ICommand<any>;
}
declare module wx {
    module internal {
        var expressionCompilerConstructor: any;
    }
}
declare module wx {
    module internal {
        var htmlTemplateEngineConstructor: any;
    }
}
declare module wx.log {
    function critical(fmt: string, ...args: any[]): void;
    function error(fmt: string, ...args: any[]): void;
    function info(fmt: string, ...args: any[]): void;
}
declare module wx {
    var messageBus: IMessageBus;
    module internal {
        var messageBusConstructor: any;
    }
}
declare module wx {
    interface IStateActiveBindingOptions {
        name: string;
        params?: Object;
    }
    module internal {
        var stateActiveBindingConstructor: any;
    }
}
declare module wx {
    interface IStateRefBindingOptions {
        name: string;
        params?: Object;
    }
    module internal {
        var stateRefBindingConstructor: any;
    }
}
declare module wx {
    module internal {
        var viewBindingConstructor: any;
    }
}
declare module wx {
    function route(route: any, rules?: any): IRoute;
}
declare module wx {
    var router: IRouter;
    module internal {
        var routerConstructor: any;
    }
}
declare module wx {
}
declare module wx {
    var version: string;
}
