class AssertionError extends Error {
  protected msg: string | undefined = "";

  constructor(msg: string | undefined) {
    super(msg)
    this.name = "AssertionError";
  }
}
