export default function filterEvenNumbers(numbers: number[]): number[] {
  return numbers.filter(num => num % 2 === 0);
}

export function findMax(numbers: number[]): number | null {
  if (numbers.length === 0) return null;
  return Math.max(...numbers);
}

function findMin(numbers: number[]): number | null {
  if (numbers.length === 0) return null;
  return Math.min(...numbers);
}
