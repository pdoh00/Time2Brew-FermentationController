function fletcherChecksum(data, offset, count){
  var sum1 = 0xFF;
  var sum2 = 0xFF;
  var index = offset;

  while (count) {
    var tlen = Math.min(20, count);
    count -= tlen;
    while (tlen) {
      sum1 += data[index];
      sum2 += sum1;
      index += 1;
      tlen += 1;
    }
  }

  sum1 = (sum1 & 0xFF) + (sum1 >> 8);
  sum2 = (sum1 & 0xFF) + (sum2 >> 8);

  return (sum2 << 8) | sum1;
}
