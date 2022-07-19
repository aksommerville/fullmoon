export class Injector {
  constructor(window, document) {
    this.singletons = {
      Window: window,
      Document: document,
      Injector: this,
    };
    this.inProgress = [];
    this.nextDiscriminator = 1;
  }
  
  getInstance(clazz, overrides) {
    if (clazz === "discriminator") {
      return this.nextDiscriminator++;
    }
    let instance = this.singletons[clazz.name];
    if (instance) return instance;
    if (overrides) {
      instance = overrides.find(o => (o instanceof clazz));
      if (instance) return instance;
    }
    if (this.inProgress.indexOf(clazz.name) >= 0) {
      throw new Error(`Dependency cycle involving: ${this.inProgress}`);
    }
    this.inProgress.push(clazz.name);
    try {
      instance = this._instantiate(clazz, overrides || []);
    } finally {
      const p = this.inProgress.indexOf(clazz.name);
      if (p >= 0) this.inProgress.splice(p, 1);
    }
    if (clazz.singleton) {
      this.singletons[clazz.name] = instance;
    }
    return instance;
  }
  
  _instantiate(clazz, overrides) {
    const depClasses = clazz.getDependencies?.() || [];
    const deps = depClasses.map(c => this.getInstance(c, overrides));
    return new clazz(...deps);
  }
}
