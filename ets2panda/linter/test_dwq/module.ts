@Sendable
export class SendableClassT<T> {}

@Sendable
export class SendableClass {}

export class NonClass {}

export function handle<T, U>(p: T) {
  let b: SendableClassT<T | NonClass>;
}

new SendableClassT<NonClass>();
