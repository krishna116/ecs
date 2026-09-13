
# ECS compiler

An ecs compiler to generate ECS(Entity Component System) code.

Features:
- A Lightweight C++ ECS System Implementation (Core Code Under 500 Lines with comment).
- Provides only a minimal interface set(unnecessary or superfluous features won't be added).
- Interface design inspired by https://github.com/skypjack/entt
- Bypass extensive use of template languages to produce code with superior readability.

## Usage

Run the compiler by provide your component declarations, for example:

```Bash
./ecs --str "struct Position{int x; int y}; struct Name{std::string name;}; class MyTag;" 
```
## Public ECS interface for example

```C++
ecs::registry_t registry;                             // Create registry.

auto entity = registry.create();                      // Create an entity.

auto& pos = registry.emplace<Position>(entity, 1, 2); // Add one component.
auto& name = registry.emplace<Name>(entity, "Jack");  // Add another component.

auto& pos = registry.patch<Position>(entity, 3, 4);   // Emplace or update component.

bool b1 = registry.has<Position>(entity);             // Check whether the component exists.

registry.erase<Position>(entity);                     // Erase the component.

for(auto& entity : registry){                         // Iterate all the entities.
  if(registry.has<Position, Name>(entity)){
    auto& pos = registry.get<Position>(entity);       // Get one component.
    auto& name = registry.get<Name>(entity);          // Get another component.
  }
}

bool b2 = registry.isValid(entity);                   // Check whether the entity is valid.
registry.destroy(entity);                             // Destroy the entity.
bool b3 = registry.isValid(entity);                   // Check whether the entity is valid again.

registry.visit(myVisitor)                             // Used to visit or serialize all the entities.

```

## How to build this ecs compiler

Install cmake and using follow commands to build:

```bash
cmake -S . -B build
cmake --build build
```